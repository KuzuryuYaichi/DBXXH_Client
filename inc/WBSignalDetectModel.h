#ifndef WBSIGNALDETECTMODEL_H
#define WBSIGNALDETECTMODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include <mutex>

#include "ipp.h"
#include "ippcore.h"
#include "ippvm.h"
#include "ipps.h"

struct SignalBaseChar
{
    int CenterFreq;
    int Bandwidth;
    bool operator==(const SignalBaseChar& other) const;
    bool operator<(const SignalBaseChar &other) const;
};

struct hash_fn
{
    std::size_t operator()(const SignalBaseChar& other) const
    {
        std::size_t h1 = std::hash<int>()(other.CenterFreq);
        std::size_t h2 = std::hash<int>()(other.Bandwidth);
        return h1 ^ h2;
    }
};

struct SignalInfo
{
    SignalBaseChar BaseInfo;
    float Snr;
    float CodeRate;
    float Amp;
};

struct DisplaySignalCharacter
{
    SignalInfo Info;
    qint64 startTime = 0; //时间戳ms
    qint64 stopTime = 0;
    bool isLegal = true;

    DisplaySignalCharacter(const SignalInfo& Info, qint64 startTime): Info(Info), startTime(startTime), stopTime(startTime) {}
};

struct ManMadeNoiseInfo
{
    int CenterFreq;
    float Amp;
    qint64 startTime = 0;
    qint64 stopTime = 0;

    ManMadeNoiseInfo(int CenterFreq, float Amp, qint64 startTime): CenterFreq(CenterFreq), Amp(Amp), startTime(startTime) {}

    bool operator==(const ManMadeNoiseInfo& other) const
    {
        if (CenterFreq != other.CenterFreq) //频点
            return false;
        if (startTime != other.startTime || stopTime != other.stopTime) //时间元素
            return false;
        return true;
    }
};

enum MODEL_USER_VIEW
{
    SIGNAL_DETECT_TABLE = 1,
    DISTURB_NOISE_TABLE = 2,
    MAN_MADE_NOISE_TABLE = 3
};

class NoiseAmp
{
public:
    int freqPointPos = 0;
    int amp = 0;

    NoiseAmp(int freqPointPos, int amp): freqPointPos(freqPointPos), amp(amp) {}

    bool operator<(const NoiseAmp& other) const //仅用于判断当前幅度值的关系
    {
        if (this->amp < other.amp)
            return true;
        else if (this->amp == other.amp)
            return this->freqPointPos < other.freqPointPos;
        return false;
    }
};

struct ManMadeNoiseAnalyse
{
    //典型频点-------测量频点0----测量电平0
    //           |---测量频点1
    long m_lGetAmpTimes = 0;
    std::map<int, std::map<int, int>> m_mapStoreAmpValueToGetManMadeNoiseValue;  //记录当前典型频率点对应的10个测量频点，包含每次分析所得的电平幅值，每次都与前一次算均值
};

class WBSignalDetectModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WBSignalDetectModel(QObject *parent = nullptr);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setUserViewType(MODEL_USER_VIEW newEUserViewType);
    void setBandwidthThreshold(uint newBandwidthThreshold);
    void setActiveThreshold(uint newActiveThreshold);
    void setFreqPointThreshold(uint newFreqPointThreshold);

    const std::map<int, std::map<int, int>>& lstTypicalFreq() const;
    void setLstTypicalFreq(const std::list<int>& newLstTypicalFreq);
    std::unordered_map<int, int> mapExistTypicalFreqNoiseRecordAmount() const;
    bool bIsDetecting() const;

    void findPeakCyclically(Ipp32f *FFtAvg, int length, unsigned long long Freqency, unsigned long long BandWidth);
    void findNoiseCharaAroundTypicalFreq(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);

signals:
    void sigTriggerRefreshData();

public slots:
    //更新表格中数据，用户在选择当前信号是否合法时不进行界面更新
    void UpdateData();
    void SetStartTime();
    void SetStopTime();

    void setAmplThreshold(float newFThreshold);
    //控制进入合法频点设置阶段flag，正在修改合法频点属性时不更新界面元素，完成修改时更新map
    void SlotTriggerLegalFreqSet(bool checked);
    //清理当前map中全部数据，开始重新计算
    void SlotCleanUp();
    //管理合法频点设置
    bool SlotImportLegalFreqConf();
    bool SlotExportLegalFreqConf();
    void setMapTypicalFreqAndItsTestFreq(const std::list<int>&);

private:
    void getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(Ipp32f *FFtAvg, int length, int Freqency, int BandWidth);
    QTimer* m_pSignalActiveChecker;
    //积累下来信号 key: SignalBaseChar ，头部节点为首次识别到该频点时的记录，需频点与带宽都满足区分阈值内才能算作有效信号
    //TODO: 可能需要长时间使用，需要设置一个处理中间数据的行为，可以利用1s的定时器，定时清理map中的list的中段数据，仅保留头尾元素
    std::unordered_map<SignalBaseChar, std::list<DisplaySignalCharacter>, hash_fn> m_mapValidSignalCharacter;
    //用于统计人为噪声（底噪）在各个频点时间内的表现形式 key: 典型频率点
    std::map<int, std::list<ManMadeNoiseInfo>> m_mapManMadeNoiseCharacter;

    MODEL_USER_VIEW m_eUserViewType;
    std::vector<std::vector<QVariant>> m_DisplayData;
    QFont m_Font;
    int m_iFullBandWidth;       //总带宽       HZ

    qint64 m_i64SystemStartTime = 0;                //系统启动时的时间，软件启动时开始计算，用于计算信号占用率
    qint64 m_i64SystemStopTime = 0;                 //系统/信号分析 停止时的时间，用于计算信号占用率，也作为当前处理的停止时间
    qint64 m_i64CurrentDetectStartTime = 0;         //当前信号处理的开始时间

    uint m_ActiveThreshold = 10;         //单位为s
    bool m_bIsSettingLegalFreqFlag = false;

    ManMadeNoiseAnalyse m_ManMadNoiseAnalyse;
    qint64 m_iFindNoiseCharaTimeGap = 0;
    std::mutex m_mutex;
    bool m_bIsDetecting = true /*false*/;            //用于记录当前是否正在检测信号
    static constexpr int LIST_LIMIT = 50;
};

#endif // WBSIGNALDETECTMODEL_H
