#include "inc/WBSignalDetectModel.h"
#include <QDateTime>
#include <QTimer>
#include <QSettings>

//考虑使用全局量记录频点识别门限以及带宽识别门限
int g_FreqPointThreshold = 10000; //单位为Hz
int g_BandwidthThreshold = 10000; //单位为Hz
float g_AmplThreshold = -60; //dBm

WBSignalDetectModel::WBSignalDetectModel(QObject *parent): QAbstractTableModel(parent)
{
    m_Font.setFamily("Microsoft Yahei");
    connect(this, &WBSignalDetectModel::sigTriggerRefreshData, this, &WBSignalDetectModel::UpdateData);
    m_pSignalActiveChecker = new QTimer(this);
    m_pSignalActiveChecker->setInterval(1000);
    connect(m_pSignalActiveChecker, &QTimer::timeout, this, &WBSignalDetectModel::UpdateData);
    m_pSignalActiveChecker->start();
    setLstTypicalFreq({ (int)2e6, (int)2.5e6, (int)5e6, (int)10e6, (int)15e6, (int)20e6, (int)25e6 });
}

QVariant WBSignalDetectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (m_eUserViewType == SIGNAL_DETECT_TABLE)
        {
            static constexpr const char* HEADER_LABEL[] = { "序号", "中心频率(MHz)", "电平(dBuV)", "带宽(MHz)", "起始时间", "结束时间", "占用带宽", "信号占用度", "合法信号" };
            return (section < 0 || section > 8)? "": HEADER_LABEL[section];
        }
        else if (m_eUserViewType == DISTURB_NOISE_TABLE)
        {
            static constexpr const char* HEADER_LABEL[] = { "序号", "中心频率(MHz)", "大信号电平(dBuV)", "起始时间", "结束时间" };
            return (section < 0 || section > 4)? "": HEADER_LABEL[section];
        }
        else if (m_eUserViewType == MAN_MADE_NOISE_TABLE)
        {
            static constexpr const char* HEADER_LABEL[] = { "典型频率点(MHz)", "测量频率(MHz)", "起始时间", "结束时间", "测量电平(dBuV)" };
            return (section < 0 || section > 4)? "": HEADER_LABEL[section];
        }
    }
    return QVariant();
}

int WBSignalDetectModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid()? 0: (int)m_DisplayData.size();
}

int WBSignalDetectModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        //三个表格同时只能显示其中一个，根据当前使用者表格类型进行显示
        //TODO:后续可能需要三个表格同时显示，则采用对col进行hide的方式实现
        if (m_eUserViewType == DISTURB_NOISE_TABLE)
            return 5;
        if (m_eUserViewType == SIGNAL_DETECT_TABLE)
            return 9;
        if (m_eUserViewType == MAN_MADE_NOISE_TABLE)
            return 5;
    }
    return 0;
}

bool WBSignalDetectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        m_DisplayData[index.row()][index.column()] = value;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags WBSignalDetectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == 8 && m_eUserViewType == SIGNAL_DETECT_TABLE && m_bIsSettingLegalFreqFlag)      //仅信号选择表格的最后一列，且当前处于正在编辑合法信号的暂停状态时可编辑
        flags |= Qt::ItemIsEditable;
    return flags;
}

QVariant WBSignalDetectModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole) //显示内容
        {
            if (index.column() == 8)
                return m_DisplayData[index.row()][index.column()].toBool()? "是": "否";
            else
                return m_DisplayData[index.row()][index.column()];
        }
        else if (role == Qt::TextAlignmentRole) //内容排版
            return Qt::AlignCenter;
        else if (role == Qt::FontRole) //字体
            return m_Font;
    }
    return QVariant();
}

void WBSignalDetectModel::SlotTriggerLegalFreqSet(bool checked)
{
    if (m_eUserViewType != SIGNAL_DETECT_TABLE) //仅供signalDetecttable使用
    {
        m_bIsSettingLegalFreqFlag = false;
        return;
    }
    m_bIsSettingLegalFreqFlag = checked;
    if (!checked)
    {
        for (const auto& signalIndex: m_DisplayData) //完成修改后根据已修改状态修改map中对应数据的值
        {
            if (signalIndex.size() >= 9)
            {
                //LZMK:反向获取用于查询map的key，存在一些问题 //TODO:当显示频点和带宽的数据的单位发生变化时需要同时变更
                //显示时使用单位为MHz
                int centerFreq = signalIndex.at(1).toDouble() * 1e6;
                int bandWidth = signalIndex.at(3).toDouble() * 1e6;
                SignalBaseChar curKey;
                curKey.Bandwidth = bandWidth;
                curKey.CenterFreq = centerFreq;
                for (auto& [key, value]: m_mapValidSignalCharacter)
                {
                    if (key.CenterFreq == curKey.CenterFreq && key.Bandwidth == curKey.Bandwidth)
                    {
                        value.front().isLegal = signalIndex.at(8).toBool();
                        break;
                    }
                }
            }
        }
    }
}

void WBSignalDetectModel::SlotCleanUp()
{
    m_mapValidSignalCharacter.clear(); //直接清理，不存在跨线程访问的问题
}

bool WBSignalDetectModel::SlotImportLegalFreqConf()
{
    QSettings legalSetting("legalFreq.ini",  QSettings::IniFormat);
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

bool WBSignalDetectModel::SlotExportLegalFreqConf()
{
    QSettings legalSetting("legalFreq.ini",  QSettings::IniFormat);
    QString groupName("IllegalFreqGroup");
    int groupIndex = 0;
    for (auto& [key, value]: m_mapValidSignalCharacter)
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

void WBSignalDetectModel::findPeakCyclically(Ipp32f* FFtAvg, int length, unsigned long long StartFreq, unsigned long long BandWidth)
{
    if (!m_bIsDetecting)
        return;
    if (m_i64SystemStartTime == 0)
    {
        m_i64SystemStartTime = QDateTime::currentMSecsSinceEpoch();
        m_i64SystemStopTime = m_i64SystemStartTime;
        m_i64CurrentDetectStartTime = m_i64SystemStartTime;
    }
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
    for (const auto& curInfo: m_lstSignalInfo) // LZMK: 不再区分曾有现停的情况，用户从信号检测表格选择合法信号仅针对单个信号特征进行检测，无所谓停止后再出现
    {
        bool foundFlag = false; // 两种情况：当前信号 1、从未有过  2、曾经有过
        for (auto& [key, value]: m_mapValidSignalCharacter)
        {
            if (curInfo.BaseInfo == key) //有过记录，更新记录
            {
                foundFlag = true;
                value.back().Info = curInfo;
                value.back().stopTime = QDateTime::currentMSecsSinceEpoch();
                break;
            }
        }
        if (!foundFlag && m_mapValidSignalCharacter.size() < LIST_LIMIT) //从未有过：增加map中的键值对，新增累计list
        {
            m_mapValidSignalCharacter[curInfo.BaseInfo] = { DisplaySignalCharacter(curInfo, QDateTime::currentMSecsSinceEpoch()) };
        }
    }
}

void WBSignalDetectModel::getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(Ipp32f *FFtAvg, int length, int StartFreq, int BandWidth)
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
            int currentNoiseAmp = noiseAmpLst[index].amp;
            int existNoiseAmp = Ampl;
            int currentNoiseAmpAvg = (existNoiseAmp * m_ManMadNoiseAnalyse.m_lGetAmpTimes + currentNoiseAmp) / (m_ManMadNoiseAnalyse.m_lGetAmpTimes + 1);
            Ampl = currentNoiseAmpAvg;
        }
    }
    ++m_ManMadNoiseAnalyse.m_lGetAmpTimes;
}

void WBSignalDetectModel::findNoiseCharaAroundTypicalFreq(Ipp32f *FFtAvg, int length, int StartFreq, int BandWidth)
{
    if (!m_bIsDetecting)
        return;
    if (m_i64SystemStartTime == 0)
    {
        m_i64SystemStartTime = QDateTime::currentMSecsSinceEpoch();
        m_i64SystemStopTime = m_i64SystemStartTime;
        m_i64CurrentDetectStartTime = m_i64SystemStartTime;
    }
    m_iFullBandWidth = BandWidth;

    qint64 nowtime = QDateTime::currentMSecsSinceEpoch(); //1s执行一次
    if (nowtime - m_iFindNoiseCharaTimeGap <= 1000)
        return;
    m_iFindNoiseCharaTimeGap = nowtime;
    getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(FFtAvg, length, StartFreq, BandWidth);

    for (const auto& [typicalFreq, m]: m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue)
    {
        std::list<ManMadeNoiseInfo> lstStarter;
        for (const auto& [testFreq, Ampl]: m) //当前噪声的起始时间使用开始分析的时间
        {
            lstStarter.emplace_back(ManMadeNoiseInfo(testFreq, Ampl, m_i64CurrentDetectStartTime));
        }
        if (!m_mapManMadeNoiseCharacter.contains(typicalFreq)) //初次处理
        {
            m_mapManMadeNoiseCharacter[typicalFreq] = lstStarter;
        }
        else
        {
            bool foundExistUnstopedTestFreq = false;
            auto& ManMadeNoiseInfoList = m_mapManMadeNoiseCharacter[typicalFreq];
            for (auto& info: ManMadeNoiseInfoList) //遍历当前典型频点下已有的list中的元素，将未设置终止时间的元素直接替换掉
            {
                for (auto& inner: lstStarter)
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
                ManMadeNoiseInfoList.splice(ManMadeNoiseInfoList.end(), lstStarter);
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

bool WBSignalDetectModel::bIsDetecting() const
{
    return m_bIsDetecting;
}

void WBSignalDetectModel::setMapTypicalFreqAndItsTestFreq(const std::list<int>& lstValue)
{
    setLstTypicalFreq(lstValue);
}

std::unordered_map<int, int> WBSignalDetectModel::mapExistTypicalFreqNoiseRecordAmount() const
{
    std::unordered_map<int, int> ExistTypicalFreqNoiseRecordAmount;
    for (const auto& [key, value]: m_mapManMadeNoiseCharacter)
        ExistTypicalFreqNoiseRecordAmount[key] = value.size();
    return ExistTypicalFreqNoiseRecordAmount;
}

const std::map<int, std::map<int, int>>& WBSignalDetectModel::lstTypicalFreq() const
{
    return m_ManMadNoiseAnalyse.m_mapStoreAmpValueToGetManMadeNoiseValue;
}

void WBSignalDetectModel::setLstTypicalFreq(const std::list<int>& newLstTypicalFreq)
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

void WBSignalDetectModel::setAmplThreshold(float newFThreshold)
{
    g_AmplThreshold = newFThreshold;
}

void WBSignalDetectModel::setFreqPointThreshold(uint newFreqPointThreshold)
{
    g_FreqPointThreshold = newFreqPointThreshold;
}

void WBSignalDetectModel::setActiveThreshold(uint newActiveThreshold)
{
    m_ActiveThreshold = newActiveThreshold;
}

void WBSignalDetectModel::setBandwidthThreshold(uint newBandwidthThreshold)
{
    g_BandwidthThreshold = newBandwidthThreshold;
}

void WBSignalDetectModel::setUserViewType(MODEL_USER_VIEW newEUserViewType)
{
    m_eUserViewType = newEUserViewType;
}

void WBSignalDetectModel::UpdateData()
{
    if (m_bIsSettingLegalFreqFlag)
        return;
//    std::lock_guard<std::mutex> lk(m_mutex);
    beginResetModel();
    m_DisplayData.clear();
    if (m_eUserViewType == SIGNAL_DETECT_TABLE)
    {
        int index = 1;
        for (auto& [key, value]: m_mapValidSignalCharacter)
        {
            std::vector<QVariant> line(9);
            line[0] = QString("%1").arg(index++);
            //LZMK: 此处将HZ转为MHZ显示有可能造成后面设置合法信号时无法反向找回到map中对应的那条数据，可能存在风险
            line[1] = QString::number(double(key.CenterFreq) / 1e6, 'f', 6);
            line[2] = QString::number(value.back().Info.Amp + 107);        //电平，采用107算法
            line[3] = QString::number(double(value.back().Info.BaseInfo.Bandwidth) / 1e6, 'f', 6);        //带宽
            QDateTime time;
            if (value.front().startTime) //起始时间
            {
                time.setMSecsSinceEpoch(value.front().startTime);
                line[4] = time.toString("MM-dd hh:mm:ss");
            }
            if (value.back().stopTime) //结束时间
            {
                time.setMSecsSinceEpoch(value.back().stopTime);
                line[5] = time.toString("MM-dd hh:mm:ss");
            }
            if (m_iFullBandWidth > 0 && m_iFullBandWidth >= value.back().Info.BaseInfo.Bandwidth)
                line[6] = QString("%1%").arg(100 * double(value.back().Info.BaseInfo.Bandwidth) / double(m_iFullBandWidth));        //占用带宽

            qint64 duringTime = 0;
            qint64 nowTime = QDateTime::currentMSecsSinceEpoch();
            for (const auto& curSigInfoInList: value) //计算一个确定频点带宽特征的信号在持续的总时间长度
            {
                if (curSigInfoInList.stopTime == 0)
                {
                    duringTime += nowTime - curSigInfoInList.startTime;
                    break;
                }
                duringTime += curSigInfoInList.stopTime - curSigInfoInList.startTime;
            }
            line[7] = QString("%1%").arg(100 * double(duringTime) / //信号占用度
                    ((m_i64SystemStopTime == m_i64SystemStartTime)? nowTime - m_i64SystemStartTime: m_i64SystemStopTime - m_i64SystemStartTime));
            line[8] = value.front().isLegal; //当前频点的信号是否合法记录在每个数据链的头部元素中
            m_DisplayData.emplace_back(std::move(line));
        }
    }
    else if (m_eUserViewType == DISTURB_NOISE_TABLE)
    {
        int index = 1;
        for (auto& [key, value]: m_mapValidSignalCharacter)
        {
            if (value.front().isLegal)
                continue;
            std::vector<QVariant> line(5);
            line[0] = QString("%1").arg(index++);
            line[1] = QString::number(double(key.CenterFreq) / double(1e6), 'f', 6); //干扰信号测量表格
            line[2] = QString::number(value.back().Info.Amp + 107); //电平，采用107算法
            QDateTime time;
            if (value.front().startTime)
            {
                time.setMSecsSinceEpoch(value.front().startTime);
                line[3] = time.toString("MM-dd hh:mm:ss"); //起始时间
            }
            if (value.back().stopTime)
            {
                time.setMSecsSinceEpoch(value.back().stopTime);
                line[4] = time.toString("MM-dd hh:mm:ss"); //结束时间
            }
            m_DisplayData.emplace_back(std::move(line));
        }
    }
    else if (m_eUserViewType == MAN_MADE_NOISE_TABLE)
    {
        for (const auto& [curNoiseFreq, value]: m_mapManMadeNoiseCharacter) //电磁环境人为噪声电平测量表格 对人为噪声统计map进行统计处理
        {
            for (const auto& restoredNoiseCharacter: value)
            {
                std::vector<QVariant> line(5);
                line[0] = (QString::number(double(curNoiseFreq) / double(1e6), 'f', 6));
                line[1] = (QString::number(double(restoredNoiseCharacter.CenterFreq) / double(1e6), 'f', 6));
                QDateTime time;
                if (restoredNoiseCharacter.startTime)
                {
                    time.setMSecsSinceEpoch(restoredNoiseCharacter.startTime);
                    line[2] = time.toString("MM-dd hh:mm:ss"); //起始时间
                }
                if (restoredNoiseCharacter.stopTime)
                {
                    time.setMSecsSinceEpoch(restoredNoiseCharacter.stopTime);
                    line[3] = (time.toString("MM-dd hh:mm:ss")); //结束时间
                }
                line[4] = (QString::number(restoredNoiseCharacter.Amp + 107));
                m_DisplayData.emplace_back(std::move(line));
            }
        }
    }
    endResetModel();
}

void WBSignalDetectModel::SetStartTime()
{
    m_i64CurrentDetectStartTime = QDateTime::currentMSecsSinceEpoch();
    m_ManMadNoiseAnalyse.m_lGetAmpTimes = 0;
    m_bIsDetecting = true;
}

void WBSignalDetectModel::SetStopTime()
{
    m_bIsDetecting = false;
    m_i64SystemStopTime = QDateTime::currentMSecsSinceEpoch();
    for (auto& [typicalFreq, value]: m_mapManMadeNoiseCharacter) //停止处理时更新人为噪声中包含记录的结束时间
    {
        for (auto& info: value)
        {
            if (info.stopTime == 0)
            {
                info.stopTime = m_i64SystemStopTime;
            }
        }
    }
}

bool SignalBaseChar::operator==(const SignalBaseChar &other) const
{
    return std::abs(CenterFreq - other.CenterFreq) < g_FreqPointThreshold /*&& std::abs(Bandwidth - other.Bandwidth) < g_BandwidthThreshold*/;
}

bool SignalBaseChar::operator<(const SignalBaseChar &other) const
{
    return (std::abs(CenterFreq - other.CenterFreq) < g_FreqPointThreshold /*&& Bandwidth < other.Bandwidth*/) || CenterFreq < other.CenterFreq;
}
