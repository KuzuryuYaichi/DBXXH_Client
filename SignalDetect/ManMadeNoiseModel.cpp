#include "SignalDetect/ManMadeNoiseModel.h"

#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>

#include "QXlsx/xlsxdocument.h"

#include "global.h"

ManMadeNoiseModel::ManMadeNoiseModel(QObject *parent): WBSignalDetectModel(parent)
{
    qRegisterMetaType<std::list<int>>("std::list<int>");
    setLstTypicalFreq({ (int)2e6, (int)2.5e6, (int)5e6, (int)10e6, (int)15e6, (int)20e6, (int)25e6 });
}

int ManMadeNoiseModel::rowCount(const QModelIndex &index) const
{
    return index.isValid()? 0: (int)m_DisplayData.size();
}

bool ManMadeNoiseModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        m_DisplayData[index.row()][index.column()] = value;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant ManMadeNoiseModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
            return m_DisplayData[index.row()][index.column()];
        else if (role == Qt::TextAlignmentRole)
            return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant ManMadeNoiseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return (section < 0 || section >= LENGTH)? "": HEADER_LABEL[section];
    }
    return QVariant();
}

int ManMadeNoiseModel::columnCount(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return LENGTH;
    }
    return 0;
}

Qt::ItemFlags ManMadeNoiseModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;;
}

void ManMadeNoiseModel::getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(Ipp32f *FFtAvg, int length, int StartFreq, int BandWidth)
{
    for (auto& [typicalFreq, m]: m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue)
    {
        for (auto& [testFreq, Ampl]: m)
        {
            std::vector<NoiseAmp> noiseAmpLst;
            //每次计算间隔为左右0.04m
            int noiseInterval = std::ceil(double(0.04e6) / double(BandWidth) * length);
            int testFreqIndex = std::ceil(double(testFreq - StartFreq) / double(BandWidth) * length);
            for (int index = testFreqIndex - std::ceil(double(noiseInterval) / 2); index < testFreqIndex + std::ceil(double(noiseInterval) / 2); ++index)
            {
                if (index < 0 || index >= length)
                    break;
                noiseAmpLst.emplace_back(NoiseAmp(double(index) / double(length) * BandWidth + StartFreq, FFtAvg[index]));
            }
            //采用20%处理法，选取幅值电平由小到大前20%的信号的中位数(10%)位置的值，作为要找的目标频点的幅值，将其更新给记录人为噪声的map中的对应元素
            std::sort(noiseAmpLst.begin(), noiseAmpLst.end());
            auto index = floor(double(noiseAmpLst.size()) / 10);
            if (index < 0 || index >= noiseAmpLst.size())
                continue;
            Ampl = (Ampl * m_ManMadNoiseAnalyse.m_lGetAmpTimes + noiseAmpLst[index].amp) / (m_ManMadNoiseAnalyse.m_lGetAmpTimes + 1);
        }
    }
    ++m_ManMadNoiseAnalyse.m_lGetAmpTimes;
}

void ManMadeNoiseModel::findNoiseCharaAroundTypicalFreq(Ipp32f *FFtAvg, int length, int StartFreq, int BandWidth)
{
    if (!m_bIsDetecting)
        return;
    m_Bandwidth = BandWidth / length;
    TimeRecord();
    auto nowtime = QDateTime::currentMSecsSinceEpoch(); //1s执行一次
    if (nowtime - m_iFindNoiseCharaTimeGap <= 1000)
        return;
    m_iFindNoiseCharaTimeGap = nowtime;
    getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(FFtAvg, length, StartFreq, BandWidth);

    std::lock_guard<std::mutex> lk(m_mutex);
    for (auto iter = m_mapManMadeNoiseCharacter.begin(); iter != m_mapManMadeNoiseCharacter.end();)
    {
        if (!m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue.contains(iter->first))
            iter = m_mapManMadeNoiseCharacter.erase(iter);
        else
            ++iter;
    }
    for (const auto& [typicalFreq, m]: m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue)
    {
        std::list<ManMadeNoiseInfo> lstStarter;
        for (const auto& [testFreq, Ampl]: m) //当前噪声的起始时间使用开始分析的时间
        {
            lstStarter.emplace_back(ManMadeNoiseInfo(testFreq, Ampl, m_i64CurrentDetectStartTime));
        }
        if (!m_mapManMadeNoiseCharacter.contains(typicalFreq))
        {
            m_mapManMadeNoiseCharacter[typicalFreq] = std::move(lstStarter);
        }
        else
        {
            bool foundExistUnstopedTestFreq = false;
            auto& ManMadeNoiseInfoList = m_mapManMadeNoiseCharacter[typicalFreq];
            for (auto& info: ManMadeNoiseInfoList) //遍历当前典型频点下已有的list中的元素，将未设置终止时间的元素直接替换掉
            {
                for (const auto& inner: lstStarter)
                {
                    if (inner == info/* && info.stopTime == 0*/)
                    {
                        info = inner;
                        info.stopTime = nowtime;
                        foundExistUnstopedTestFreq = true;
                        break;
                    }
                }
            }
            if (!foundExistUnstopedTestFreq) //终止之后再开始，整体添加当前的lst
            {
                for (auto& info: lstStarter)
                {
                    info.startTime = m_i64CurrentDetectStartTime;
                }
                ManMadeNoiseInfoList.splice(ManMadeNoiseInfoList.end(), std::move(lstStarter));
            }
        }
        //连续处理四个小时后，下次进入新一轮分析处理
        if (nowtime - m_mapManMadeNoiseCharacter[typicalFreq].back().startTime >= 4 * 3600 * 1000)
        {
            for (auto& info: m_mapManMadeNoiseCharacter[typicalFreq])
            {
                info.stopTime = nowtime;
            }
            m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
            m_i64CurrentDetectStartTime = nowtime;
        }
    }
}

const std::map<int, std::map<int, double>>& ManMadeNoiseModel::lstTypicalFreq() const
{
    return m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue;
}

void ManMadeNoiseModel::setLstTypicalFreq(const std::list<int>& newLstTypicalFreq)
{
    m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue.clear();
    m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
    for (const auto& typicalFreq: newLstTypicalFreq)
    {
        std::map<int, double> testFreqAndItsAmp;
        for (int index = 0; index < 10; ++index)
        {
            int currentTestFreq = typicalFreq - 0.2e6 + 0.02e6 + index * 0.04e6;
            if (currentTestFreq < 0 || currentTestFreq > 30e6)
                continue;
            testFreqAndItsAmp[currentTestFreq] = 0;
        }
        if (!testFreqAndItsAmp.empty())
            m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue[typicalFreq] = testFreqAndItsAmp;
    }
}

void ManMadeNoiseModel::UpdateData()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    beginResetModel();
    m_DisplayData.clear();
    for (const auto& [curNoiseFreq, value]: m_mapManMadeNoiseCharacter)
    {
        for (const auto& restoredNoiseCharacter: value)
        {
            std::vector<QVariant> line(LENGTH);
            line[0] = QString::number(curNoiseFreq / 1e6, 'f', 6);
            line[1] = QString::number(restoredNoiseCharacter.CenterFreq / 1e6, 'f', 6);
            if (restoredNoiseCharacter.startTime)
                line[2] = QDateTime::fromMSecsSinceEpoch(restoredNoiseCharacter.startTime).toString(TIME_FORMAT);
            if (restoredNoiseCharacter.stopTime)
                line[3] = QDateTime::fromMSecsSinceEpoch(restoredNoiseCharacter.stopTime).toString(TIME_FORMAT);
            line[4] = QString::number(restoredNoiseCharacter.Amp + 107, 'f', 1);
            m_DisplayData.emplace_back(std::move(line));
        }
    }
    endResetModel();
}

void ManMadeNoiseModel::SetTime(bool isDetecting)
{
    WBSignalDetectModel::SetTime(isDetecting);
    std::lock_guard<std::mutex> lk(m_mutex);
    if (isDetecting)
    {
        m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
    }
}

bool ManMadeNoiseModel::GenerateExcelManMadeNoiseTable(const CommonInfoSet& CommonInfo)
{
    QFileDialog dialog;
    auto folderName = dialog.getExistingDirectory(nullptr, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderName.isEmpty())
    {
        qDebug() << "File saving cancelled.";
        return false;
    }

    QXlsx::Document xlsx;

    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::CellRange range("B2:E2");
    xlsx.mergeCells(range, format);
    xlsx.write("B2", QString("测量日期：%1").arg(CommonInfo.Date.toString(" yyyy年 MM月 dd日")), format);

    range = QXlsx::CellRange("F2:K2");
    xlsx.mergeCells(range, format);
    xlsx.write("F2", QString("测试地点：%1       北纬：°N     东经：°E	").arg(CommonInfo.TestPosition), format);

    xlsx.write("B3", "环境条件", format);

    range = QXlsx::CellRange("C3:E3");
    xlsx.mergeCells(range, format);
    xlsx.write("C3", QString("天气状况：%1    温度：%2℃          湿度：%3%rh			").arg(CommonInfo.Weather).arg(CommonInfo.Temprature).arg(CommonInfo.Humidity), format);

    xlsx.write("F3", "测量仪器", format);

    range = QXlsx::CellRange("G3:K3");
    xlsx.mergeCells(range, format);
    xlsx.write("G3", "短波接收天线 短波测量仪", format);

    // Write column headers
    xlsx.write("B4", "典型频率点(MHz)", format);
    xlsx.write("C4", "测量频率(MHz)", format);
    xlsx.write("D4", "起始时间", format);
    xlsx.write("E4", "结束时间", format);
    xlsx.write("F4", "测量电平(dBuV)", format);
    xlsx.write("G4", "平均电平(dBuV)", format);
    xlsx.write("H4", "最大电平(dBuV)", format);
    xlsx.write("I4", "最小电平(dBuV)", format);
    xlsx.write("J4", "检波方式", format);
    xlsx.write("K4", "中频带宽", format);

    //填写界面上相应的数据
    int dataPosRow = 5;
    std::list<int> existAmpForEveryTypicalFreqLst;
    QModelIndex item;
    for (int row = 0; row < rowCount(); ++row)
    {
        int curDataCol = 1;
        item = index(row, curDataCol++); //测量频率
        if (item.isValid())
            xlsx.write("C" + QString::number(dataPosRow), data(item), format);

        item = index(row, curDataCol++); //起始时间
        if (item.isValid())
            xlsx.write(QString("D") + QString::number(dataPosRow), data(item), format);

        item = index(row, curDataCol++); //结束时间
        if (item.isValid())
            xlsx.write(QString("E") + QString::number(dataPosRow), data(item), format);

        item = index(row, curDataCol++); //测量电平
        if (item.isValid())
            xlsx.write(QString("F") + QString::number(dataPosRow), data(item), format);

        existAmpForEveryTypicalFreqLst.emplace_back(data(item).toString().toDouble());
        ++dataPosRow;
    }

    //计算平均电平、最大电平、最小电平，合并对应单元格并填入
    size_t startRow = 5;
    for (const auto& [curTypicalFreq, curNoiseRecord]: m_mapManMadeNoiseCharacter)
    {
        auto curNoiseRecordAmount = curNoiseRecord.size();
        //先将典型频率点占用的单元格合并
        range = QXlsx::CellRange("B" + QString::number(startRow) + ":B" + QString::number(startRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("B" + QString::number(startRow), QString::number(double(curTypicalFreq) / 1e6, 'f', 6), format);

        std::list<int> existAmpForCurrentTypicalFreqLst;
        int totalAmpValue = 0;
        for (int num = 0; num < curNoiseRecordAmount; ++num)
        {
            totalAmpValue += existAmpForEveryTypicalFreqLst.front();
            existAmpForCurrentTypicalFreqLst.emplace_back(std::move(existAmpForEveryTypicalFreqLst.front()));
            existAmpForEveryTypicalFreqLst.pop_front();
        }

        //平均电平
        range = QXlsx::CellRange("G" + QString::number(startRow) + ":G" + QString::number(startRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("G" + QString::number(startRow), QString::number(double(totalAmpValue) / curNoiseRecordAmount, 'f', 1), format);
        //最大电平
        range = QXlsx::CellRange("H" + QString::number(startRow) + ":H" + QString::number(startRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("H" + QString::number(startRow), QString::number(*std::max_element(existAmpForCurrentTypicalFreqLst.begin(), existAmpForCurrentTypicalFreqLst.end())), format);
        //最小电平
        range = QXlsx::CellRange("I" + QString::number(startRow) + ":I" + QString::number(startRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("I" + QString::number(startRow), QString::number(*std::min_element(existAmpForCurrentTypicalFreqLst.begin(), existAmpForCurrentTypicalFreqLst.end())), format);
        //检波方式
        range = QXlsx::CellRange("J" + QString::number(startRow) + ":J" + QString::number(startRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("J" + QString::number(startRow), "RMS", format);
        //中频带宽
        range = QXlsx::CellRange("K" + QString::number(startRow) + ":K" + QString::number(startRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("K" + QString::number(startRow), "2.4kHz", format);
        startRow += curNoiseRecordAmount;
    }
    return xlsx.saveAs(folderName + "/电磁环境人为噪声电平测量记录" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".xlsx");
}

bool ManMadeNoiseModel::GenerateWordManMadeNoiseChart(QAxObject* document)
{
    auto bookmark_ManMadeNoiseChart = document->querySubObject("Bookmarks(QVariant)", "ManMadeNoiseChart");
    if (!bookmark_ManMadeNoiseChart || bookmark_ManMadeNoiseChart->isNull())
    {
        QMessageBox::critical(nullptr, "电磁环境测试报告", "人为噪声图标模板错误");
        return false;
    }
    auto bookmark_Result = document->querySubObject("Bookmarks(QVariant)", "Result");
    if (!bookmark_Result || bookmark_Result->isNull())
    {
        QMessageBox::critical(nullptr, "电磁环境测试报告", "人为噪声图标模板错误");
        return false;
    }

    auto rowCnt = rowCount();
    std::list<int> existAmpForEveryTypicalFreqLst;
    for (int row = 0; row < rowCnt; ++row)
    {
        auto item = index(row, 4);
        if (item.isValid())
        {
            existAmpForEveryTypicalFreqLst.emplace_back(data(item).toString().toDouble());
        }
    }

    std::vector<std::pair<int, double>> avgAmpValue;
    for (const auto& [curTypicalFreq, curNoiseRecord]: m_mapManMadeNoiseCharacter)
    {
        auto curNoiseRecordAmount = curNoiseRecord.size();
        std::vector<int> existAmpForCurrentTypicalFreqLst;
        int totalAmpValue = 0;
        for (int num = 0; num < curNoiseRecordAmount; ++num)
        {
            totalAmpValue += existAmpForEveryTypicalFreqLst.front();
            existAmpForCurrentTypicalFreqLst.emplace_back(std::move(existAmpForEveryTypicalFreqLst.front()));
            existAmpForEveryTypicalFreqLst.pop_front();
        }
        avgAmpValue.emplace_back(curTypicalFreq / 1e6, double(totalAmpValue) / curNoiseRecordAmount);
    }
    if (avgAmpValue.size() < SAMPLE_POINTS)
    {
        QMessageBox::critical(nullptr, "噪音等级图", "数据缺失");
        return false;
    }
    AddSeries(document->querySubObject("InlineShapes")->querySubObject("AddChart(QVariant, QVariant)", 4,
        bookmark_ManMadeNoiseChart->querySubObject("Range")->asVariant())->querySubObject("Chart"), avgAmpValue);
    AddResult(bookmark_Result->querySubObject("Range"), avgAmpValue);
    return true;
}

bool ManMadeNoiseModel::Intersects(const QLineF& lineA, const QLineF& lineB, QPointF& interPos)
{
    return QLineF::BoundedIntersection == lineA.intersects(lineB, &interPos);
}

void ManMadeNoiseModel::AddResult(QAxObject* range, const std::vector<std::pair<int, double>>& avgAmpValue)
{
    std::vector<std::pair<double, int>> intersections;
    for (auto point = 1; point < SAMPLE_POINTS; ++point)
    {
        for (auto level = 1; level < LEVEL_LENGTH; ++level)
        {
            QLineF lineA(avgAmpValue[point - 1].first, fa[0][point - 1], avgAmpValue[point].first, fa[0][point]),
                lineB(avgAmpValue[point - 1].first, fa[level][point - 1], avgAmpValue[point].first, fa[level][point]);
            QPointF res;
            if (Intersects(lineA, lineB, res))
            {
                intersections.emplace_back(res.x(), level);
            }
        }
    }
    double level_analyze[LEVEL_LENGTH] = { 0 };
    auto len = intersections.size();
    for (auto i = 1; i < len; ++i)
    {
        auto& [freq1, level1] = intersections[i - 1];
        auto& [freq2, level2] = intersections[i];
        level_analyze[level1 < level2? level1: level2] += freq2 - freq1;
    }
    auto level = std::max_element(level_analyze, level_analyze + LEVEL_LENGTH) - level_analyze;
    if (level == 0)
    {
        for (auto point = 0; point < SAMPLE_POINTS; ++point)
        {
            for (level = LEVEL_LENGTH - 1; level > 0; --level)
            {
                if (fa[0][point] > fa[level][point])
                    break;
            }
        }
    }
    range->querySubObject("InsertAfter(QVariant)", QString("背景噪声位于%1级别\n").arg(POSITION_LEVEL[level]));
}

void ManMadeNoiseModel::AddSeries(QAxObject* chart, const std::vector<std::pair<int, double>>& avgAmpValue)
{
    auto chartWorkSheet = chart->querySubObject("chartData")->querySubObject("Workbook")->querySubObject("WorkSheets(QVariant)", 1);
    auto Range = chartWorkSheet->querySubObject("Range(QVariant)", "A1:F8");
    chartWorkSheet->querySubObject("ListObjects(QVariant)", 1)->querySubObject("Resize(QVariant)", Range->asVariant());
    for (auto i = 0; i < SAMPLE_POINTS; ++i)
    {
        chartWorkSheet->querySubObject("Range(QVariant)", "A" + QString::number(i + 2))->setProperty("FormulaR1C1", avgAmpValue[i].first);
    }
    for (auto i = 0; i < LEVEL_LENGTH; ++i)
    {
        chartWorkSheet->querySubObject("Range(QVariant)", QChar('B' + i) + QString("1"))->setProperty("FormulaR1C1", POSITION_LEVEL[i]);
    }
    for (auto point = 0; point < SAMPLE_POINTS; ++point)
    {
        auto& [freq, amp] = avgAmpValue[point];
        double af;
        auto p_af = std::lower_bound(AF, AF + AF_LENGTH, freq, [](const auto& a, const auto& b) {
            return a.first < b;
        });
        //查找最接近的AF值
        if (p_af == AF + AF_LENGTH)
            af = AF[AF_LENGTH - 1].second;
        else if (p_af == AF)
            af = AF[0].second;
        else
            af = ((std::abs(p_af->first - freq) < std::abs((p_af - 1)->first - freq))? p_af->second: (p_af - 1)->second);
        for (auto level = 0; level < LEVEL_LENGTH; ++level)
        {
            if (level == 0)
                fa[level][point] = amp + af - 20 * std::log10(freq) - 10 * std::log10(m_Bandwidth) + 95.5;
            else
                fa[level][point] = c[level - 1] - d[level - 1] * std::log10(freq);
        }
    }
    for (int level = 0; level < LEVEL_LENGTH; ++level)
    {
        for (auto point = 0; point < SAMPLE_POINTS; ++point)
        {
            chartWorkSheet->querySubObject("Range(QVariant)", QChar('B' + level) + QString::number(point + 2))->setProperty("FormulaR1C1", fa[level][point]);
        }
    }
    //    chart->querySubObject("Legend")->setProperty("Position", -4107);
}
