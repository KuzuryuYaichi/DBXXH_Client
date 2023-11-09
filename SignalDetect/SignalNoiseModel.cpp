#include "SignalDetect/SignalNoiseModel.h"

#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

#include "QXlsx/xlsxdocument.h"

#include "global.h"

SignalNoiseModel::SignalNoiseModel(QObject* parent): WBSignalDetectModel(parent) {}

int SignalNoiseModel::rowCount(const QModelIndex &index) const
{
    return index.isValid()? 0: (int)m_DisplayData.size();
}

bool SignalNoiseModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        m_DisplayData[index.row()].second[index.column()] = value;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

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

int SignalNoiseModel::columnCount(const QModelIndex &index) const
{
    if (!index.isValid())
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
    if ((m_eUserViewType == DISTURB_NOISE_TABLE && index.column() == LENGTH_DISTURB_NOISE - 1 && m_IsSettingRemarkFlag) ||
        (m_eUserViewType == SIGNAL_DETECT_TABLE && index.column() == LENGTH_SIGNAL_DETECT - 1 && m_IsSettingLegalFreqFlag))
        flags |= Qt::ItemIsEditable;
    return flags;
}

QVariant SignalNoiseModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            if (m_eUserViewType == SIGNAL_DETECT_TABLE && index.column() == LENGTH_SIGNAL_DETECT - 1)
                return m_DisplayData[index.row()].second[index.column()].toBool()? "是": "否";
            else
                return m_DisplayData[index.row()].second[index.column()];
        }
        else if (role == Qt::TextAlignmentRole)
            return Qt::AlignCenter;
    }
    return QVariant();
}

void SignalNoiseModel::TriggerLegalFreqSet(bool checked)
{
    if (!(m_IsSettingLegalFreqFlag = checked))
    {
        beginResetModel();
        for (const auto& [uuid, data]: m_DisplayData)
        {
            for (auto& [key, value]: m_mapValidSignalCharacter)
            {
                if (uuid == key.uuid)
                {
                    value.isLegal = data[LENGTH_SIGNAL_DETECT - 1].toBool();
                    break;
                }
            }
        }
        endResetModel();
    }
}

void SignalNoiseModel::TriggerRemarkSet(bool checked)
{
    if (!(m_IsSettingRemarkFlag = checked))
    {
        beginResetModel();
        for (const auto& [uuid, signalIndex]: m_DisplayData)
        {
            for (auto& [key, value]: m_mapValidSignalCharacter)
            {
                if (uuid == key.uuid)
                {
                    value.Remark = signalIndex[LENGTH_DISTURB_NOISE - 1].toString();
                    break;
                }
            }
        }
        endResetModel();
    }
}

void SignalNoiseModel::CleanUp()
{
    m_mapValidSignalCharacter.clear(); //直接清理，不存在跨线程访问的问题
    m_i64SystemStartTime = m_i64SystemStopTime = 0;
}

bool SignalNoiseModel::ImportLegalFreqConf()
{
    QSettings legalSetting("legalFreq.ini", QSettings::IniFormat);
    auto groups = legalSetting.childGroups();
    for (const auto& curGroup: groups)
    {
        legalSetting.beginGroup(curGroup);
        auto CenterFreq = legalSetting.value("CenterFreq").toInt();
        auto Bandwidth = legalSetting.value("Bandwidth").toInt();
        auto Remark = legalSetting.value("Remark").toString();
        bool findSignal = false;
        for (auto& [key, value]: m_mapValidSignalCharacter)
        {
            if (key.CenterFreq == CenterFreq && key.Bandwidth == Bandwidth)
            {
                value.isLegal = false;
                value.Remark = Remark;
                findSignal = true;
                break;
            }
        }
        if (!findSignal)
        {
            m_mapValidSignalCharacter.emplace(std::pair(SignalBaseChar(CenterFreq, Bandwidth), DisplaySignalCharacter(SignalInfo(), 0, Remark)));
        }
        legalSetting.endGroup();
    }
    return true;
}

bool SignalNoiseModel::ExportLegalFreqConf()
{
    QSettings legalSetting("legalFreq.ini", QSettings::IniFormat);
    QString groupName("IllegalFreqGroup");
    int groupIndex = 0;
    for (const auto& [key, value]: m_mapValidSignalCharacter)
    {
        if (!value.isLegal)
        {
            legalSetting.beginGroup(groupName + QString::number(groupIndex++));
            legalSetting.setValue("CenterFreq", key.CenterFreq);
            legalSetting.setValue("Bandwidth", key.Bandwidth);
            legalSetting.setValue("Remark", value.Remark);
            legalSetting.endGroup();
        }
    }
    return true;
}

void SignalNoiseModel::findPeakCyclically(Ipp32f* FFtAvg, int length, unsigned long long StartFreq, unsigned long long BandWidth)
{
    if (!m_bIsDetecting)
        return;
    TimeRecord();
    m_iFullBandWidth = BandWidth;
    //从左向右查找第一个峰值的位置
    //LZMK:对原有逻辑进行改造，原有逻辑为根据对应区间获取最大值最高点作为信号peak
    //当前采用门限进行限制，只要存在某一点比前后两个点的幅度大的情况，即可认为是一个尖峰
    std::list<std::pair<SignalBaseChar, SignalInfo>> m_lstSignalInfo;
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
        if (MaxAddr == 0)
            break;
        static constexpr auto AMPL_THRESHOLD = 15;
        //TODO: 是否需要处理两个波峰过于接近，导致没能在右侧找到6dB边界的情况
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
        Ipp32f SignalPower;
        ippsMean_32f(FFtAvg + LeftAddr, RightAddr - LeftAddr + 1, &SignalPower, ippAlgHintFast); //获取信号平均功率
        Ipp32f NoiseLeftPower;
        ippsMean_32f(FFtAvg, LeftAddr, &NoiseLeftPower, ippAlgHintFast); //获取噪声平均功率
        Ipp32f NoiseRightPower;
        ippsMean_32f(FFtAvg + RightAddr, length - RightAddr, &NoiseRightPower, ippAlgHintFast);
        totalIndex = RightAddr + 1; //更新次回处理起点
        m_lstSignalInfo.emplace_back(SignalBaseChar(StartFreq + step * (RightAddr + LeftAddr) / 2, (RightAddr - LeftAddr) * step),
                                     SignalInfo(SignalPower - (NoiseRightPower + NoiseLeftPower) / 2, FFtMax));
    }
    auto currentTime = QDateTime::currentMSecsSinceEpoch();
    for (const auto& [baseInfo, curInfo]: m_lstSignalInfo) // LZMK: 不再区分曾有现停的情况，用户从信号检测表格选择合法信号仅针对单个信号特征进行检测，无所谓停止后再出现
    {
        bool foundFlag = false; // 两种情况：当前信号 1、从未有过  2、曾经有过
        for (auto& [key, value]: m_mapValidSignalCharacter)
        {
            if (baseInfo == key) //有过记录，更新记录
            {
                foundFlag = true;
                if (value.stopTime <= currentTime - 3000)
                {
                    value.BlankTime += currentTime - value.stopTime;
                }
                value.Info = curInfo;
                value.stopTime = currentTime;
                break;
            }
        }
        if (!foundFlag && m_mapValidSignalCharacter.size() < LIST_LIMIT) //从未有过：增加map中的键值对，新增累计list
        {
            m_mapValidSignalCharacter[baseInfo] = DisplaySignalCharacter(curInfo, currentTime);
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
    if (m_IsSettingLegalFreqFlag || m_IsSettingRemarkFlag)
        return;
    beginResetModel();
    m_DisplayData.clear();
    if (m_eUserViewType == SIGNAL_DETECT_TABLE)
    {
        for (const auto& [key, value]: m_mapValidSignalCharacter)
        {
            std::vector<QVariant> line(LENGTH_SIGNAL_DETECT);
            line[0] = QString::number(double(key.CenterFreq) / 1e6, 'f', 6);
            line[1] = QString::number(value.Info.Amp + 107);
            line[2] = QString::number(double(key.Bandwidth) / 1e6, 'f', 6); //带宽
            if (value.startTime) //起始时间
                line[3] = QDateTime::fromMSecsSinceEpoch(value.startTime).toString(TIME_FORMAT);
            if (value.stopTime) //结束时间
                line[4] = QDateTime::fromMSecsSinceEpoch(value.stopTime).toString(TIME_FORMAT);
            if (m_iFullBandWidth > 0 && m_iFullBandWidth >= key.Bandwidth)
                line[5] = QString("%1%").arg(100.0 * key.Bandwidth / m_iFullBandWidth); //占用带宽

            qint64 duringTime = 0, nowTime = QDateTime::currentMSecsSinceEpoch();
            if (value.stopTime == 0) //计算一个确定频点带宽特征的信号在持续的总时间长度
                duringTime += nowTime - value.startTime;
            else
                duringTime += value.stopTime - value.startTime - value.BlankTime;
            line[6] = QString("%1%").arg(100.0 * duringTime / ((m_i64SystemStopTime == m_i64SystemStartTime)?
                                nowTime - m_i64SystemStartTime: m_i64SystemStopTime - m_i64SystemStartTime));
            line[7] = value.isLegal; //当前频点的信号是否合法记录在每个数据链的头部元素中
            m_DisplayData.emplace_back(std::pair{ key.uuid, std::move(line) });
        }
    }
    else if (m_eUserViewType == DISTURB_NOISE_TABLE)
    {
        for (const auto& [key, value]: m_mapValidSignalCharacter)
        {
            if (value.isLegal)
                continue;
            std::vector<QVariant> line(LENGTH_DISTURB_NOISE);
            line[0] = QString::number(key.CenterFreq / 1e6, 'f', 6); //干扰信号测量表格
            line[1] = QString::number(value.Info.Amp + 107);
            if (value.startTime)
                line[2] = QDateTime::fromMSecsSinceEpoch(value.startTime).toString(TIME_FORMAT); //起始时间
            if (value.stopTime)
                line[3] = QDateTime::fromMSecsSinceEpoch(value.stopTime).toString(TIME_FORMAT); //结束时间
            line[4] = value.Remark; //说明
            m_DisplayData.emplace_back(std::pair{ key.uuid, std::move(line) });
        }
    }
    endResetModel();
}

bool SignalNoiseModel::GenerateExcelSignalDetectTable()
{
    QFileDialog dialog;
    auto folderName = dialog.getExistingDirectory(nullptr, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderName.isEmpty())
    {
        qDebug() << "File saving cancelled.";
        return false;
    }

    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);
    QXlsx::Document xlsx;
    for (int col = 0; col < columnCount(); ++col)
    {
        xlsx.write(2, col + 2, QString(HEADER_LABEL_SIGNAL_DETECT[col]), format);
    }
    for (int row = 0; row < rowCount(); ++row)
    {
        for (int col = 0; col < columnCount(); ++col)
        {
            auto item = index(row, col);
            if (item.isValid())
                xlsx.write(row + 3, col + 2, data(item), format);
        }
    }
    return xlsx.saveAs(folderName + "/信号检测列表" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".xlsx");
}

bool SignalNoiseModel::GenerateExcelDisturbNoiseTable(const CommonInfoSet& CommonInfo)
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
    QXlsx::CellRange range("B2:D2");
    xlsx.mergeCells(range, format);
    xlsx.write("B2", QString("测量日期：%1").arg(CommonInfo.Date.toString(" yyyy年 MM月 dd日")), format);

    range = QXlsx::CellRange("E2:I2");
    xlsx.mergeCells(range, format);
    xlsx.write("E2", QString("测试地点：%1       北纬：°N     东经：°E	").arg(CommonInfo.TestPosition), format);

    xlsx.write("B3", "环境条件", format);

    range = QXlsx::CellRange("C3:I3");
    xlsx.mergeCells(range, format);
    xlsx.write("C3", QString("天气状况：%1    温度：%2℃          湿度：%3%rh			").arg(CommonInfo.Weather).arg(CommonInfo.Temprature).arg(CommonInfo.Humidity), format);

    xlsx.write("B4", "测量仪器", format);

    range = QXlsx::CellRange("C4:D4");
    xlsx.mergeCells(range, format);
    xlsx.write("C4", "短波接收天线 短波测量仪", format);

    xlsx.write("E4", "中频带宽", format);
    xlsx.write("F4", "2.4kHz", format);

    xlsx.write("G4", "检波方式", format);

    range = QXlsx::CellRange("H4:I4");
    xlsx.mergeCells(range, format);
    xlsx.write("H4", "RMS", format);
    xlsx.write("B5", QString(HEADER_LABEL_DISTURB_NOISE[0]), format);

    range = QXlsx::CellRange("C5:D5");
    xlsx.mergeCells(range, format);
    xlsx.write("C5", QString(HEADER_LABEL_DISTURB_NOISE[1]), format);
    xlsx.write("E5", QString(HEADER_LABEL_DISTURB_NOISE[2]), format);
    xlsx.write("F5", QString(HEADER_LABEL_DISTURB_NOISE[3]), format);

    range = QXlsx::CellRange("G5:I5");
    xlsx.mergeCells(range, format);
    xlsx.write("G5", "说明", format);

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
        item = index(row, curDataCol++);
        if (item.isValid())
            xlsx.write(QString("G") + QString::number(dataPosRow), data(item), format);

        ++dataPosRow;
    }
    return xlsx.saveAs(folderName + "/干扰信号测量记录" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".xlsx");
}
