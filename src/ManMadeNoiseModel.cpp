#include "inc/ManMadeNoiseModel.h"

#include <QDateTime>
#include <QMessageBox>

#include "global.h"

ManMadeNoiseModel::ManMadeNoiseModel(QObject *parent): WBSignalDetectModel(parent)
{
    setLstTypicalFreq({ (int)2e6, (int)2.5e6, (int)5e6, (int)10e6, (int)15e6, (int)20e6, (int)25e6 });
}

QVariant ManMadeNoiseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return (section < 0 || section >= LENGTH)? "": HEADER_LABEL[section];
    }
    return QVariant();
}

int ManMadeNoiseModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
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
            int noiseInterval = ceil(double(0.04e6) / double(BandWidth) * length);
            int testFreqIndex = ceil(double(testFreq - StartFreq) / double(BandWidth) * length);
            for (int index = testFreqIndex - ceil(double(noiseInterval) / 2); index < testFreqIndex + ceil(double(noiseInterval) / 2); ++index)
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
    TimeRecord();
    qint64 nowtime = QDateTime::currentMSecsSinceEpoch(); //1s执行一次
    if (nowtime - m_iFindNoiseCharaTimeGap <= 1000)
        return;
    m_iFindNoiseCharaTimeGap = nowtime;
    getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(FFtAvg, length, StartFreq, BandWidth);

    std::lock_guard<std::mutex> lk(m_mutex);
    for (const auto& [typicalFreq, m]: m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue)
    {
        std::list<ManMadeNoiseInfo> lstStarter;
        for (const auto& [testFreq, Ampl]: m) //当前噪声的起始时间使用开始分析的时间
        {
            lstStarter.emplace_back(ManMadeNoiseInfo(testFreq, Ampl, m_i64CurrentDetectStartTime));
        }
        if (!m_mapManMadeNoiseCharacter.contains(typicalFreq)) //初次处理
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
                    if (inner == info && info.stopTime == 0)
                    {
                        info = inner;
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

std::map<int, int> ManMadeNoiseModel::mapExistTypicalFreqNoiseRecordAmount() const
{
    std::map<int, int> ExistTypicalFreqNoiseRecordAmount;
    for (const auto& [key, value]: m_mapManMadeNoiseCharacter)
        ExistTypicalFreqNoiseRecordAmount[key] = value.size();
    return ExistTypicalFreqNoiseRecordAmount;
}

const std::map<int, std::map<int, int>>& ManMadeNoiseModel::lstTypicalFreq() const
{
    return m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue;
}

void ManMadeNoiseModel::setLstTypicalFreq(const std::list<int>& newLstTypicalFreq)
{
    m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue.clear();
    m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
    //通过初始化典型频点，计算当前典型频点及其各自的测试频点
    for (const auto& typicalFreq: newLstTypicalFreq)
    {
        std::map<int, int> testFreqAndItsAmp;
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
    for (const auto& [curNoiseFreq, value]: m_mapManMadeNoiseCharacter) //电磁环境人为噪声电平测量表格 对人为噪声统计map进行统计处理
    {
        for (const auto& restoredNoiseCharacter: value)
        {
            std::vector<QVariant> line(LENGTH);
            line[0] = QString::number(double(curNoiseFreq) / double(1e6), 'f', 6);
            line[1] = QString::number(double(restoredNoiseCharacter.CenterFreq) / double(1e6), 'f', 6);
            if (restoredNoiseCharacter.startTime)
                line[2] = QDateTime::fromMSecsSinceEpoch(restoredNoiseCharacter.startTime).toString(TIME_FORMAT); //起始时间
            if (restoredNoiseCharacter.stopTime)
                line[3] = QDateTime::fromMSecsSinceEpoch(restoredNoiseCharacter.stopTime).toString(TIME_FORMAT); //结束时间
            line[4] = QString::number(restoredNoiseCharacter.Amp + 107);
            m_DisplayData.emplace_back(std::move(line));
        }
    }
    endResetModel();
}

//void ManMadeNoiseModel::SetStartTime()
//{
//    m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
//    WBSignalDetectModel::SetStartTime();
//}

//void ManMadeNoiseModel::SetStopTime()
//{
//    WBSignalDetectModel::SetStopTime();
//    for (auto& [typicalFreq, value]: m_mapManMadeNoiseCharacter) //停止处理时更新人为噪声中包含记录的结束时间
//    {
//        for (auto& info: value)
//        {
//            if (info.stopTime == 0)
//            {
//                info.stopTime = m_i64SystemStopTime;
//            }
//        }
//    }
//}

bool ManMadeNoiseModel::GenerateExcelManMadeNoiseTable(QString folderName)
{
    auto amount = mapExistTypicalFreqNoiseRecordAmount();
    QString fileName = folderName + "/电磁环境人为噪声电平测量记录" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".xlsx";
    QXlsx::Document xlsx;
    //不跟随当前实际信号状态递增的部分
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::CellRange range("B2:E2");
    xlsx.mergeCells(range, format);
    xlsx.write("B2", "测量日期和时间：     年   月   日		", format);

    range = QXlsx::CellRange("F2:K2");
    xlsx.mergeCells(range, format);
    xlsx.write("F2", "测试地点：         北纬：       东经：				", format);

    xlsx.write("B3", "环境条件", format);

    range = QXlsx::CellRange("C3:E3");
    xlsx.mergeCells(range, format);
    xlsx.write("C3", "天气状况：      温度：           湿度：						", format);

    xlsx.write("F3", "测量仪器", format);

    range = QXlsx::CellRange("G3:K3");
    xlsx.mergeCells(range, format);
    xlsx.write("G3", "", format);

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

    // Write table data
    //先将典型频率点占用的单元格合并
    int startRow = 5;
    for (const auto& [curTypicalFreq, curNoiseRecordAmount]: amount)
    {
        range = QXlsx::CellRange("B" + QString::number(startRow) + ":B" + QString::number(startRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("B" + QString::number(startRow), QString::number(double(curTypicalFreq) / 1e6, 'f', 6), format);
        startRow += curNoiseRecordAmount;
    }

    //填写界面上相应的数据
    int dataPosRow = 5;
    QList<int> existAmpForEveryTypicalFreqLst;
    QModelIndex item;
    for (int row = 0; row < rowCount(); ++row)
    {
        int curDataCol = 1;
        item = index(row, curDataCol++);       //测量频率
        if (item.isValid())
            xlsx.write("C" + QString::number(dataPosRow), data(item), format);

        item = index(row, curDataCol++);           //起始时间
        if (item.isValid())
            xlsx.write(QString("D") + QString::number(dataPosRow), data(item), format);

        item = index(row, curDataCol++);           //结束时间
        if (item.isValid())
            xlsx.write(QString("E") + QString::number(dataPosRow), data(item), format);

        item = index(row, curDataCol++);           //测量电平
        if (item.isValid())
            xlsx.write(QString("F") + QString::number(dataPosRow), data(item), format);

        existAmpForEveryTypicalFreqLst.append(data(item).toString().toDouble());
        ++dataPosRow;
    }

    //计算平均电平、最大电平、最小电平，合并对应单元格并填入
    int staticStartRow = 5;
    for (const auto& [curTypicalFreq, curNoiseRecordAmount]: amount)
    {
        QList<int> existAmpForCurrentTypicalFreqLst;
        int totalAmpValue = 0;
        for (int num = 0; num < curNoiseRecordAmount; ++num)
        {
            existAmpForCurrentTypicalFreqLst.append(existAmpForEveryTypicalFreqLst.takeFirst());
            totalAmpValue += existAmpForCurrentTypicalFreqLst.constLast();
        }

        //平均电平
        range = QXlsx::CellRange("G" + QString::number(staticStartRow) + ":G" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("G" + QString::number(staticStartRow), QString::number(double(totalAmpValue) / curNoiseRecordAmount, 'f', 1), format);

        std::sort(existAmpForCurrentTypicalFreqLst.begin(), existAmpForCurrentTypicalFreqLst.end());

        //最大电平
        range = QXlsx::CellRange("H" + QString::number(staticStartRow) + ":H" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("H" + QString::number(staticStartRow), QString::number(existAmpForCurrentTypicalFreqLst.constLast()), format);
        //最小电平
        range = QXlsx::CellRange("I" + QString::number(staticStartRow) + ":I" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("I" + QString::number(staticStartRow), QString::number(existAmpForCurrentTypicalFreqLst.constFirst()), format);
        //检波方式
        range = QXlsx::CellRange("J" + QString::number(staticStartRow) + ":J" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("J" + QString::number(staticStartRow), "RMS", format);
        //中频带宽
        range = QXlsx::CellRange("K" + QString::number(staticStartRow) + ":K" + QString::number(staticStartRow + curNoiseRecordAmount - 1));
        xlsx.mergeCells(range, format);
        xlsx.write("K" + QString::number(staticStartRow), "2.4kHz", format);
        staticStartRow += curNoiseRecordAmount;
    }
    return xlsx.saveAs(fileName);
}

bool ManMadeNoiseModel::GenerateWordManMadeNoiseTable(QAxObject* document)
{
    auto amount = mapExistTypicalFreqNoiseRecordAmount();
    auto bookmark_table = document->querySubObject("Bookmarks(QVariant)", "ManMadeNoise");
    if (!bookmark_table || bookmark_table->isNull())
    {
        QMessageBox::critical(nullptr, "电磁环境测试报告", "模板错误");
        return false;
    }
    auto rowCnt = rowCount(), colCnt = 10;
    auto datatable = document->querySubObject("Tables")->querySubObject("Add(QAxObject*, int, int, QVariant&, QVariant&)",
                    bookmark_table->querySubObject("Range")->asVariant(), rowCnt + 4, colCnt);
    datatable->setProperty("Style", "网格型");

    MergeCells(datatable, 1, 6, 1, colCnt);
    MergeCells(datatable, 1, 1, 1, 5);
    MergeCells(datatable, 2, 7, 2, colCnt);
    MergeCells(datatable, 2, 2, 2, 5);
    MergeCells(datatable, rowCnt + 4, 2, rowCnt + 4, colCnt);

    auto rangeTitle = datatable->querySubObject("Cell(int, int)", 1, 1)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "测量日期：     年     月    日");

    rangeTitle = datatable->querySubObject("Cell(int, int)", 1, 2)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "测量地点：     北纬：         东经：         ");

    rangeTitle = datatable->querySubObject("Cell(int, int)", 2, 1)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "环境条件");

    rangeTitle = datatable->querySubObject("Cell(int, int)", 2, 2)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "天气状况：  温度：  湿度：  ");

    rangeTitle = datatable->querySubObject("Cell(int, int)", 2, 3)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "测量仪器");

    rangeTitle = datatable->querySubObject("Cell(int, int)", rowCnt + 4, 1)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "测量人员");

    static QStringList HEADER_LIST
        { "典型频率点(MHz)", "测量频率(MHz)", "起始时间", "结束时间", "测量电平(dBuV)", "平均电平(dBuV)", "最大电平(dBuV)", "最小电平(dBuV)", "检波方式", "中频带宽" };
    for (int col = 0; col < colCnt; ++col)
    {
        auto rangeTitle = datatable->querySubObject("Cell(int, int)", 3, col + 1)->querySubObject("Range");
        rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
        rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
        rangeTitle->dynamicCall("SetText(QString)", HEADER_LIST[col]);
    }

    std::list<int> existAmpForEveryTypicalFreqLst; //填写界面上相应的数据
    for (int row = 0; row < rowCnt; ++row)
    {
        for (int col = 0; col < 5; ++col)
        {
            auto item = index(row, col); //测量频率 起始时间 结束时间 测量电平
            if (item.isValid())
            {
                auto rangeTitle = datatable->querySubObject("Cell(int, int)", row + 4, col + 1)->querySubObject("Range");
                rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
                rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
                rangeTitle->dynamicCall("SetText(QString)", data(item).toString());
                if (col == 4)
                    existAmpForEveryTypicalFreqLst.emplace_back(data(item).toString().toDouble());
            }
        }
    }

    //计算平均电平 最大电平 最小电平 合并对应单元格并填入
    for (const auto& [curTypicalFreq, curNoiseRecordAmount]: amount)
    {
        std::vector<int> existAmpForCurrentTypicalFreqLst;
        int totalAmpValue = 0;
        for (int num = 0; num < curNoiseRecordAmount; ++num)
        {
            auto& amp = existAmpForEveryTypicalFreqLst.front();
            totalAmpValue += amp;
            existAmpForCurrentTypicalFreqLst.emplace_back(std::move(amp));
            existAmpForEveryTypicalFreqLst.pop_front();
        }

        for (int row = 0; row < rowCnt; ++row)
        {
            for (int col = 5; col < colCnt; ++col)
            {
                auto rangeTitle = datatable->querySubObject("Cell(int, int)", row + 4, col + 1)->querySubObject("Range");
                rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
                rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
                switch (col)
                {
                case 5: rangeTitle->dynamicCall("SetText(QString)", QString::number(double(totalAmpValue) / curNoiseRecordAmount, 'f', 1)); break;
                case 6: rangeTitle->dynamicCall("SetText(QString)", QString::number(*std::max_element(existAmpForCurrentTypicalFreqLst.begin(), existAmpForCurrentTypicalFreqLst.end()))); break;
                case 7: rangeTitle->dynamicCall("SetText(QString)", QString::number(*std::min_element(existAmpForCurrentTypicalFreqLst.begin(), existAmpForCurrentTypicalFreqLst.end()))); break;
                case 8: rangeTitle->dynamicCall("SetText(QString)", "RMS"); break;
                case 9: rangeTitle->dynamicCall("SetText(QString)", "2.4kHz"); break;
                }
            }
        }
    }
    return true;
}

bool ManMadeNoiseModel::GenerateWordManMadeNoiseChart(QAxObject* document)
{
    auto bookmark_ManMadeNoiseChart = document->querySubObject("Bookmarks(QVariant)", "ManMadeNoiseChart");
    if (!bookmark_ManMadeNoiseChart || bookmark_ManMadeNoiseChart->isNull())
    {
        QMessageBox::critical(nullptr, "电磁环境测试报告", "模板错误");
        return false;
    }
    auto bookmark_Result = document->querySubObject("Bookmarks(QVariant)", "Result");
    if (!bookmark_Result || bookmark_Result->isNull())
    {
        QMessageBox::critical(nullptr, "电磁环境测试报告", "模板错误");
        return false;
    }

    auto amount = mapExistTypicalFreqNoiseRecordAmount();
    auto rowCnt = rowCount();
    std::list<int> existAmpForEveryTypicalFreqLst; //填写界面上相应的数据
    for (int row = 0; row < rowCnt; ++row)
    {
        auto item = index(row, 4); //测量频率 起始时间 结束时间 测量电平
        if (item.isValid())
        {
            existAmpForEveryTypicalFreqLst.emplace_back(data(item).toString().toDouble());
        }
    }

    std::vector<double> avgAmpValue;
    //计算平均电平 最大电平 最小电平 合并对应单元格并填入
    for (const auto& [curTypicalFreq, curNoiseRecordAmount]: amount)
    {
        std::vector<int> existAmpForCurrentTypicalFreqLst;
        int totalAmpValue = 0;
        for (int num = 0; num < curNoiseRecordAmount; ++num)
        {
            auto& amp = existAmpForEveryTypicalFreqLst.front();
            totalAmpValue += amp;
            existAmpForCurrentTypicalFreqLst.emplace_back(std::move(amp));
            existAmpForEveryTypicalFreqLst.pop_front();
        }
        avgAmpValue.emplace_back(double(totalAmpValue) / curNoiseRecordAmount);
    }
    if (avgAmpValue.size() < SAMPLE_POINTS)
    {
        QMessageBox::critical(nullptr, "噪音等级图", "数据缺失");
        return false;
    }
    AddSeries(document->querySubObject("InlineShapes")->querySubObject("AddChart(QVariant, QVariant)",
            4, bookmark_ManMadeNoiseChart->querySubObject("Range")->asVariant())->querySubObject("Chart"), avgAmpValue);
    AddResult(bookmark_Result->querySubObject("Range"));
    return true;
}

void ManMadeNoiseModel::AddResult(QAxObject* range)
{
    QString Result;
    for (auto point = 0; point < SAMPLE_POINTS; ++point)
    {
        int level;
        for (level = 1; level < LEVEL_LENGTH; ++level)
        {
            if (f_am[0][point] < f_am[level][point])
                break;
        }
        Result += QString("%1MHz背景噪声位于%2%3级别\n").arg(SAMPLE_FREQ[point]).arg(level == 1? "银河": POSITION_LEVEL[level - 1]).
                                arg(level == LEVEL_LENGTH? "": QString("到%1").arg(POSITION_LEVEL[level]));
    }
    range->querySubObject("InsertAfter(QVariant)", Result);
}

void ManMadeNoiseModel::AddSeries(QAxObject* chart, const std::vector<double>& avgAmpValue)
{
    auto chartWorkSheet = chart->querySubObject("chartData")->querySubObject("Workbook")->querySubObject("WorkSheets(QVariant)", 1);
    auto Range = chartWorkSheet->querySubObject("Range(QVariant)", "A1:F8");
    chartWorkSheet->querySubObject("ListObjects(QVariant)", 1)->querySubObject("Resize(QVariant)", Range->asVariant());
    for (auto i = 0; i < SAMPLE_POINTS; ++i)
    {
        chartWorkSheet->querySubObject("Range(QVariant)", "A" + QString::number(i + 2))->setProperty("FormulaR1C1", SAMPLE_FREQ[i]);
    }
    for (auto i = 0; i < LEVEL_LENGTH; ++i)
    {
        chartWorkSheet->querySubObject("Range(QVariant)", QChar('B' + i) + QString("1"))->setProperty("FormulaR1C1", POSITION_LEVEL[i]);
    }

    f_am = F_AM();
    for (auto point = 0; point < SAMPLE_POINTS; ++point)
    {
        f_am[0][point] = avgAmpValue[point];
    }
    for (int level = 0; level < LEVEL_LENGTH; ++level)
    {
        for (auto point = 0; point < SAMPLE_POINTS; ++point)
        {
            chartWorkSheet->querySubObject("Range(QVariant)", QChar('B' + level) + QString::number(point + 2))->setProperty("FormulaR1C1", f_am[level][point]);
        }
    }
    //    chart->querySubObject("Legend")->setProperty("Position", -4107);
}
