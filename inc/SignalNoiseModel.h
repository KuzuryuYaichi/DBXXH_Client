#ifndef SIGNALNOISEMODEL_H
#define SIGNALNOISEMODEL_H

#include "WBSignalDetectModel.h"

struct SignalBaseChar
{
    int CenterFreq;
    int Bandwidth;
    SignalBaseChar() = default;
    SignalBaseChar(int CenterFreq, int Bandwidth): CenterFreq(CenterFreq), Bandwidth(Bandwidth) {}
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
    SignalInfo() = default;
    SignalInfo(float Snr, float CodeRate, float Amp): Snr(Snr), CodeRate(CodeRate), Amp(Amp) {}
};

struct DisplaySignalCharacter
{
    SignalInfo Info;
    qint64 startTime; //时间戳ms
    qint64 stopTime;
    qint64 BlankTime = 0;
    bool isLegal = true;

    DisplaySignalCharacter(const SignalInfo& Info, qint64 startTime): Info(Info), startTime(startTime), stopTime(startTime) {}
};

enum MODEL_USER_VIEW
{
    SIGNAL_DETECT_TABLE,
    DISTURB_NOISE_TABLE
};

class SignalNoiseModel: public WBSignalDetectModel
{
    Q_OBJECT
public:
    explicit SignalNoiseModel(QObject* = nullptr);
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int = Qt::DisplayRole) const override;

    void setUserViewType(MODEL_USER_VIEW);
    void setBandwidthThreshold(uint);
    void setActiveThreshold(uint);
    void setFreqPointThreshold(uint);
    void setAmplThreshold(float);

    bool GenerateExcelSignalDetectTable(QString);
    bool GenerateExcelDisturbNoiseTable(QString);
    bool GenerateWordDisturbNoiseTable(QAxObject*);

    void findPeakCyclically(Ipp32f*, int, unsigned long long, unsigned long long);

public slots:
    //更新表格中数据，用户在选择当前信号是否合法时不进行界面更新
    void UpdateData() override;
    //控制进入合法频点设置阶段flag，正在修改合法频点属性时不更新界面元素，完成修改时更新map
    void SlotTriggerLegalFreqSet(bool checked);
    //清理当前map中全部数据，开始重新计算
    void SlotCleanUp();
    //管理合法频点设置
    bool SlotImportLegalFreqConf();
    bool SlotExportLegalFreqConf();

private:
    //积累下来信号 key: SignalBaseChar ，头部节点为首次识别到该频点时的记录，需频点与带宽都满足区分阈值内才能算作有效信号
    //TODO: 可能需要长时间使用，需要设置一个处理中间数据的行为，可以利用1s的定时器，定时清理map中的list的中段数据，仅保留头尾元素
    std::map<SignalBaseChar, std::list<DisplaySignalCharacter>> m_mapValidSignalCharacter;

    MODEL_USER_VIEW m_eUserViewType;
    int m_iFullBandWidth;
    bool m_bIsSettingLegalFreqFlag = false;

    static constexpr const char* HEADER_LABEL_SIGNAL_DETECT[] = { "中心频率(MHz)", "电平(dBuV)", "带宽(MHz)", "起始时间", "结束时间", "占用带宽", "信号占用度", "合法信号" },
                        * HEADER_LABEL_DISTURB_NOISE[] = { "中心频率(MHz)", "大信号电平(dBuV)", "起始时间", "结束时间" };
    static constexpr auto LENGTH_SIGNAL_DETECT = sizeof(HEADER_LABEL_SIGNAL_DETECT) / sizeof(HEADER_LABEL_SIGNAL_DETECT[0]),
                        LENGTH_DISTURB_NOISE = sizeof(HEADER_LABEL_DISTURB_NOISE) / sizeof(HEADER_LABEL_DISTURB_NOISE[0]);
    static constexpr int LIST_LIMIT = 50;
};

#endif // SIGNALNOISEMODEL_H
