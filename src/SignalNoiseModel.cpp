#include "inc/SignalNoiseModel.h"

#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>

#include "global.h"

//考虑使用全局量记录频点识别门限以及带宽识别门限
uint g_FreqPointThreshold = 10000; //单位为Hz
uint g_BandwidthThreshold = 10000; //单位为Hz
uint g_ActiveThreshold = 10;
float g_AmplThreshold = -60; //dBm

SignalNoiseModel::SignalNoiseModel(QObject* parent): WBSignalDetectModel(parent) {}

QVariant SignalNoiseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (m_eUserViewType == SIGNAL_DETECT_TABLE)
        {
            return (section < 0 || section >= LENGTH_SIGNAL_DETECT)? "": HEADER_LABEL_SIGNAL_DETECT[section];
        }
        else if (m_eUserViewType == DISTURB_NOISE_TABLE)
        {
            return (section < 0 || section >= LENGTH_DISTURB_NOISE)? "": HEADER_LABEL_DISTURB_NOISE[section];
        }
    }
    return QVariant();
}

int SignalNoiseModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        if (m_eUserViewType == SIGNAL_DETECT_TABLE)
            return LENGTH_SIGNAL_DETECT;
        if (m_eUserViewType == DISTURB_NOISE_TABLE)
            return LENGTH_DISTURB_NOISE;
    }
    return 0;
}

Qt::ItemFlags SignalNoiseModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == LENGTH_SIGNAL_DETECT - 1 && m_eUserViewType == SIGNAL_DETECT_TABLE && m_bIsSettingLegalFreqFlag)      //仅信号选择表格的最后一列，且当前处于正在编辑合法信号的暂停状态时可编辑
        flags |= Qt::ItemIsEditable;
    return flags;
}

QVariant SignalNoiseModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole) //显示内容
        {
            if (index.column() == LENGTH_SIGNAL_DETECT - 1)
                return m_DisplayData[index.row()][index.column()].toBool()? "是": "否";
            else
                return m_DisplayData[index.row()][index.column()];
        }
        else if (role == Qt::TextAlignmentRole) //内容排版
            return Qt::AlignCenter;
    }
    return QVariant();
}

void SignalNoiseModel::SlotTriggerLegalFreqSet(bool checked)
{
    if (!(m_bIsSettingLegalFreqFlag = checked))
    {
        for (const auto& signalIndex: m_DisplayData) //完成修改后根据已修改状态修改map中对应数据的值
        {
            if (signalIndex.size() >= LENGTH_SIGNAL_DETECT)
            {
                //LZMK:反向获取用于查询map的key，存在一些问题 //TODO:当显示频点和带宽的数据的单位发生变化时需要同时变更
                //显示时使用单位为MHz
                SignalBaseChar curKey(signalIndex[0].toDouble() * 1e6, signalIndex[2].toDouble() * 1e6);
                for (auto& [key, value]: m_mapValidSignalCharacter)
                {
                    if (key.CenterFreq == curKey.CenterFreq && key.Bandwidth == curKey.Bandwidth)
                    {
                        value.front().isLegal = signalIndex[LENGTH_SIGNAL_DETECT - 1].toBool();
                        break;
                    }
                }
            }
        }
    }
}

void SignalNoiseModel::SlotCleanUp()
{
    m_mapValidSignalCharacter.clear(); //直接清理，不存在跨线程访问的问题
    m_i64SystemStartTime = m_i64SystemStopTime = 0;
}

bool SignalNoiseModel::SlotImportLegalFreqConf()
{
    QSettings legalSetting("legalFreq.ini", QSettings::IniFormat);
    auto groups = legalSetting.childGroups();
    for (const auto& curGroup: groups)
    {
        legalSetting.beginGroup(curGroup);
        int centerFreq = legalSetting.value("centerFreq").toInt();
        int bandWidth = legalSetting.value("bandWidth").toInt();
        for (auto& [key, value]: m_mapValidSignalCharacter)
        {
            if (key.CenterFreq == centerFreq && key.Bandwidth == bandWidth)
            {
                value.front().isLegal = false;
                break;
            }
        }
        legalSetting.endGroup();
    }
    return true;
}

bool SignalNoiseModel::SlotExportLegalFreqConf()
{
    QSettings legalSetting("legalFreq.ini", QSettings::IniFormat);
    QString groupName("IllegalFreqGroup");
    int groupIndex = 0;
    for (const auto& [key, value]: m_mapValidSignalCharacter)
    {
        if (!value.front().isLegal)
        {
            legalSetting.beginGroup(groupName + QString::number(groupIndex++));
            legalSetting.setValue("centerFreq", value.front().Info.BaseInfo.CenterFreq);
            legalSetting.setValue("bandWidth", value.front().Info.BaseInfo.Bandwidth);
            legalSetting.endGroup();
        }
    }
    return true;
}

void SignalNoiseModel::findPeakCyclically(Ipp32f* FFtAvg, int length, unsigned long long StartFreq, unsigned long long BandWidth)
{
    TimeRecord();
    m_iFullBandWidth = BandWidth;
    //从左向右查找第一个峰值的位置
    //LZMK:对原有逻辑进行改造，原有逻辑为根据对应区间获取最大值最高点作为信号peak；
    //当前采用门限进行限制，只要存在某一点比前后两个点的幅度大的情况，即可认为是一个尖峰
    std::list<SignalInfo> m_lstSignalInfo;
    for (int totalIndex = 0; totalIndex < length;)
    {
        Ipp32f FFtMax = g_AmplThreshold;
        int MaxAddr = 0;
        int LeftAddr = 0;
        int RightAddr = length;
        for (auto index = totalIndex + 1; index < length - 1; ++index)
        {
            if (FFtAvg[index] <= g_AmplThreshold || FFtAvg[index + 1] >= FFtAvg[index] || FFtAvg[index] <= FFtAvg[index - 1])
                continue;
            FFtMax = FFtAvg[index];
            MaxAddr = index;
            break;
        }
        if (MaxAddr == 0) //找不到最大点了就可以退出了
            break;
        static constexpr auto AMPL_THRESHOLD = 15;
        //TODO: 是否需要处理两个波峰过于接近，导致没能在右侧找到6dB边界的情况？
        for (auto index = MaxAddr; index < length - 1; ++index) //处理出现未找到6dB右边界的情况且包络走势出现上扬的趋势时直接作为右边界
        {
            RightAddr = index;
            if (/*FFtAvg[index + 1] > FFtAvg[index] &&*/ FFtMax - FFtAvg[index] > AMPL_THRESHOLD)
                break;
        }
        for (auto index = MaxAddr; index > totalIndex; --index) //按照最高峰位置反向搜索信号 寻找左边界
        {
            LeftAddr = index;//获取左边界
            if (/*FFtAvg[index - 1] < FFtAvg[index] &&*/ FFtMax - FFtAvg[index] > AMPL_THRESHOLD)
                break;
        }
        auto step = (double)BandWidth / length;
        SignalInfo currentSignalInfo; //计算信号属性
        currentSignalInfo.BaseInfo.Bandwidth = (RightAddr - LeftAddr) * step;
        currentSignalInfo.BaseInfo.CenterFreq = StartFreq + step * (RightAddr + LeftAddr) / 2;
        currentSignalInfo.Amp = FFtMax;
        Ipp32f SignalPower;
        ippsMean_32f(&FFtAvg[LeftAddr], RightAddr - LeftAddr + 1, &SignalPower, ippAlgHintFast); //获取信号平均功率
        Ipp32f NoiseLeftPower;
        ippsMean_32f(FFtAvg, LeftAddr, &NoiseLeftPower, ippAlgHintFast); //获取噪声平均功率
        Ipp32f NoiseRightPower;
        ippsMean_32f(&FFtAvg[RightAddr], length - RightAddr, &NoiseRightPower, ippAlgHintFast);
        currentSignalInfo.Snr = SignalPower - (NoiseRightPower + NoiseLeftPower) / 2;
        totalIndex = RightAddr + 1; //更新次回处理起点
        m_lstSignalInfo.emplace_back(currentSignalInfo);
    }
    auto currentTime = QDateTime::currentMSecsSinceEpoch();
    for (const auto& curInfo: m_lstSignalInfo) // LZMK: 不再区分曾有现停的情况，用户从信号检测表格选择合法信号仅针对单个信号特征进行检测，无所谓停止后再出现
    {
        bool foundFlag = false; // 两种情况：当前信号 1、从未有过  2、曾经有过
        for (auto& [key, value]: m_mapValidSignalCharacter)
        {
            if (curInfo.BaseInfo == key) //有过记录，更新记录
            {
                foundFlag = true;
                if (value.back().stopTime <= currentTime - 3000)
                {
                    value.back().BlankTime += currentTime - value.back().stopTime;
                }
                value.back().Info = curInfo;
                value.back().stopTime = currentTime;
                break;
            }
        }
        if (!foundFlag && m_mapValidSignalCharacter.size() < LIST_LIMIT) //从未有过：增加map中的键值对，新增累计list
        {
            m_mapValidSignalCharacter[curInfo.BaseInfo] = { DisplaySignalCharacter(curInfo, currentTime) };
        }
    }
}

void SignalNoiseModel::setAmplThreshold(float newFThreshold)
{
    g_AmplThreshold = newFThreshold;
}

void SignalNoiseModel::setFreqPointThreshold(uint newFreqPointThreshold)
{
    g_FreqPointThreshold = newFreqPointThreshold;
}

void SignalNoiseModel::setActiveThreshold(uint newActiveThreshold)
{
    g_ActiveThreshold = newActiveThreshold;
}

void SignalNoiseModel::setBandwidthThreshold(uint newBandwidthThreshold)
{
    g_BandwidthThreshold = newBandwidthThreshold;
}

void SignalNoiseModel::setUserViewType(MODEL_USER_VIEW newEUserViewType)
{
    m_eUserViewType = newEUserViewType;
}

void SignalNoiseModel::UpdateData()
{
    if (m_bIsSettingLegalFreqFlag)
        return;
    //    std::lock_guard<std::mutex> lk(m_mutex);
    beginResetModel();
    m_DisplayData.clear();
    if (m_eUserViewType == SIGNAL_DETECT_TABLE)
    {
        for (const auto& [key, value]: m_mapValidSignalCharacter)
        {
            std::vector<QVariant> line(LENGTH_SIGNAL_DETECT);
            //LZMK: 此处将HZ转为MHZ显示有可能造成后面设置合法信号时无法反向找回到map中对应的那条数据，可能存在风险
            line[0] = QString::number(double(key.CenterFreq) / 1e6, 'f', 6);
            line[1] = QString::number(value.back().Info.Amp + 107);        //电平，采用107算法
            line[2] = QString::number(double(value.back().Info.BaseInfo.Bandwidth) / 1e6, 'f', 6);        //带宽
            if (value.front().startTime) //起始时间
                line[3] = QDateTime::fromMSecsSinceEpoch(value.front().startTime).toString(TIME_FORMAT);
            if (value.back().stopTime) //结束时间
                line[4] = QDateTime::fromMSecsSinceEpoch(value.back().stopTime).toString(TIME_FORMAT);
            if (m_iFullBandWidth > 0 && m_iFullBandWidth >= value.back().Info.BaseInfo.Bandwidth)
                line[5] = QString("%1%").arg(100.0 * value.back().Info.BaseInfo.Bandwidth / m_iFullBandWidth);        //占用带宽

            qint64 duringTime = 0;
            qint64 nowTime = QDateTime::currentMSecsSinceEpoch();
            for (const auto& curSigInfoInList: value) //计算一个确定频点带宽特征的信号在持续的总时间长度
            {
                if (curSigInfoInList.stopTime == 0)
                {
                    duringTime += nowTime - curSigInfoInList.startTime;
                    break;
                }
                duringTime += curSigInfoInList.stopTime - curSigInfoInList.startTime - curSigInfoInList.BlankTime;
            }
            line[6] = QString("%1%").arg(100.0 * duringTime / ((m_i64SystemStopTime == m_i64SystemStartTime)?
                                nowTime - m_i64SystemStartTime: m_i64SystemStopTime - m_i64SystemStartTime));
            line[7] = value.front().isLegal; //当前频点的信号是否合法记录在每个数据链的头部元素中
            m_DisplayData.emplace_back(std::move(line));
        }
    }
    else if (m_eUserViewType == DISTURB_NOISE_TABLE)
    {
        for (const auto& [key, value]: m_mapValidSignalCharacter)
        {
            if (value.front().isLegal)
                continue;
            std::vector<QVariant> line(LENGTH_DISTURB_NOISE);
            line[0] = QString::number(double(key.CenterFreq) / double(1e6), 'f', 6); //干扰信号测量表格
            line[1] = QString::number(value.back().Info.Amp + 107); //电平，采用107算法
            if (value.front().startTime)
                line[2] = QDateTime::fromMSecsSinceEpoch(value.front().startTime).toString(TIME_FORMAT); //起始时间
            if (value.back().stopTime)
                line[3] = QDateTime::fromMSecsSinceEpoch(value.back().stopTime).toString(TIME_FORMAT); //结束时间
            m_DisplayData.emplace_back(std::move(line));
        }
    }
    endResetModel();
}

bool SignalBaseChar::operator==(const SignalBaseChar& other) const
{
    return std::abs(CenterFreq - other.CenterFreq) < g_FreqPointThreshold /*&& std::abs(Bandwidth - other.Bandwidth) < g_BandwidthThreshold*/;
}

bool SignalBaseChar::operator<(const SignalBaseChar& other) const
{
    return /*(std::abs(CenterFreq - other.CenterFreq) < g_FreqPointThreshold && Bandwidth < other.Bandwidth) ||*/ CenterFreq < other.CenterFreq;
}

bool SignalNoiseModel::GenerateExcelSignalDetectTable(QString folderName)
{
    QString fileName(folderName + "/信号检测列表" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".xlsx");
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);
    QXlsx::Document xlsx;
    // Write column headers
    for (int col = 0; col < columnCount(); ++col)
    {
        xlsx.write(1, col + 1, QString(HEADER_LABEL_SIGNAL_DETECT[col]));
    }
    // Write table data
    for (int row = 0; row < rowCount(); ++row)
    {
        for (int col = 0; col < columnCount(); ++col)
        {
            auto item = index(row, col);
            if (item.isValid())
                xlsx.write(row + 2, col + 1, data(item), format);
        }
    }
    // Save the Excel file
    return xlsx.saveAs(fileName);
}

bool SignalNoiseModel::GenerateExcelDisturbNoiseTable(QString folderName)
{
    QString fileName = folderName + "/干扰信号测量记录" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".xlsx";

    QXlsx::Document xlsx;

    //不跟随当前实际信号状态递增的部分
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);
    QXlsx::CellRange range("B2:D2");
    xlsx.mergeCells(range, format);
    xlsx.write("B2", "测量日期和时间：     年   月   日		", format);

    range = QXlsx::CellRange("E2:I2");
    xlsx.mergeCells(range, format);
    xlsx.write("E2", "测试地点：         北纬：       东经：				", format);

    xlsx.write("B3", "环境条件", format);

    range = QXlsx::CellRange("C3:I3");
    xlsx.mergeCells(range, format);
    xlsx.write("C3", "天气状况：      温度：           湿度：						", format);

    xlsx.write("B4", "测量仪器", format);

    range = QXlsx::CellRange("C4:D4");
    xlsx.mergeCells(range, format);
    xlsx.write("C4", "", format);

    xlsx.write("E4", "中频带宽", format);
    xlsx.write("F4", "2.4kHz", format);

    xlsx.write("G4", "检波方式", format);

    range = QXlsx::CellRange("H4:I4");
    xlsx.mergeCells(range, format);
    xlsx.write("H4", "RMS", format);

    // Write column headers
    xlsx.write("B5", QString(HEADER_LABEL_DISTURB_NOISE[0]), format);

    range = QXlsx::CellRange("C5:D5");
    xlsx.mergeCells(range, format);
    xlsx.write("C5", QString(HEADER_LABEL_DISTURB_NOISE[1]), format);

    xlsx.write("E5", QString(HEADER_LABEL_DISTURB_NOISE[2]), format);

    xlsx.write("F5", QString(HEADER_LABEL_DISTURB_NOISE[3]), format);

    range = QXlsx::CellRange("G5:I5");
    xlsx.mergeCells(range, format);
    xlsx.write("G5", "说明", format);

    // Write table data
    int dataPosRow = 6;
    QModelIndex item;
    for (int row = 0; row < rowCount(); ++row)
    {
        auto curDataCol = 0;
        item = index(row, curDataCol++);
        if (item.isValid())
            xlsx.write("B" + QString::number(dataPosRow), data(item), format);

        range = QXlsx::CellRange("C" + QString::number(dataPosRow) + ":" + "D" + QString::number(dataPosRow));
        xlsx.mergeCells(range, format);
        item = index(row, curDataCol++);
        if (item.isValid())
            xlsx.write(QString("C") + QString::number(dataPosRow), data(item), format);

        item = index(row, curDataCol++);
        if (item.isValid())
            xlsx.write(QString("E") + QString::number(dataPosRow), data(item), format);

        item = index(row, curDataCol++);
        if (item.isValid())
            xlsx.write(QString("F") + QString::number(dataPosRow), data(item), format);

        range = QXlsx::CellRange("G" + QString::number(dataPosRow) + ":" + "I" + QString::number(dataPosRow));
        xlsx.mergeCells(range, format);

        ++dataPosRow;
    }

    // Save the Excel file
    return xlsx.saveAs(fileName);
}

bool SignalNoiseModel::GenerateWordDisturbNoiseTable(QAxObject* document)
{
    auto bookmark_table = document->querySubObject("Bookmarks(QVariant)", "DisturbNoise");
    if (!bookmark_table || bookmark_table->isNull())
    {
        QMessageBox::critical(nullptr, "电磁环境测试报告", "模板错误");
        return false;
    }
    auto rowCnt = rowCount();
    auto colCnt = LENGTH_DISTURB_NOISE;
    auto datatable = document->querySubObject("Tables")->querySubObject("Add(QAxObject*, int, int, QVariant&, QVariant&)",
                    bookmark_table->querySubObject("Range")->asVariant(), rowCnt + 5, colCnt);
    datatable->setProperty("Style", "网格型");

    MergeCells(datatable, 1, 3, 1, colCnt);
    MergeCells(datatable, 1, 1, 1, 2);
    MergeCells(datatable, rowCnt + 5, 2, rowCnt + 5, colCnt);

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

    rangeTitle = datatable->querySubObject("Cell(int, int)", 3, 1)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "中频带宽");

    rangeTitle = datatable->querySubObject("Cell(int, int)", 3, 3)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "检波方式");

    rangeTitle = datatable->querySubObject("Cell(int, int)", rowCnt + 5, 1)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", "测量人员");

    for (int col = 0; col < colCnt; ++col)
    {
        auto rangeTitle = datatable->querySubObject("Cell(int, int)", 4, col + 1)->querySubObject("Range");
        rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
        rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
        rangeTitle->dynamicCall("SetText(QString)", HEADER_LABEL_DISTURB_NOISE[col]);
    }
    for (int row = 0; row < rowCnt; ++row)
    {
        for (int col = 0; col < colCnt; ++col)
        {
            auto item = index(row, col);
            if (item.isValid())
            {
                auto rangeTitle = datatable->querySubObject("Cell(int, int)", row + 5, col + 1)->querySubObject("Range");
                rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
                rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
                rangeTitle->dynamicCall("SetText(QString)", data(item));
            }
        }
    }
    return true;
}

