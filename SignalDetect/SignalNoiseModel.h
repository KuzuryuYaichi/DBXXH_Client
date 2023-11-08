#ifndef SIGNALNOISEMODEL_H
#define SIGNALNOISEMODEL_H

#include "WBSignalDetectModel.h"
#include "SignalDetect/CommonInfoDialog.h"

extern int g_FreqPointThreshold;
extern int g_BandwidthThreshold;
extern float g_AmplThreshold;

struct SignalBaseChar
{
    QUuid uuid;
    int CenterFreq;
    int Bandwidth;
    SignalBaseChar(): uuid(QUuid::createUuid()) {}
    SignalBaseChar(int CenterFreq, int Bandwidth): uuid(QUuid::createUuid()), CenterFreq(CenterFreq), Bandwidth(Bandwidth) {}
    bool operator==(const SignalBaseChar& other) const
    {
        return std::abs(CenterFreq - other.CenterFreq) < g_FreqPointThreshold /*&& std::abs(Bandwidth - other.Bandwidth) < g_BandwidthThreshold*/;
    }
    bool operator<(const SignalBaseChar& other) const
    {
        return /*(std::abs(CenterFreq - other.CenterFreq) < g_FreqPointThreshold && Bandwidth < other.Bandwidth) ||*/ CenterFreq < other.CenterFreq;
    }
};

struct SignalInfo
{
    float Snr = 0;
    float Amp = 0;
    SignalInfo() = default;
    SignalInfo(float Snr, float Amp): Snr(Snr), Amp(Amp) {}
};

struct DisplaySignalCharacter
{
    SignalInfo Info;
    qint64 startTime;
    qint64 stopTime;
    qint64 BlankTime = 0;
    QString Remark;
    bool isLegal = true;

    DisplaySignalCharacter() = default;
    DisplaySignalCharacter(const SignalInfo& Info, qint64 startTime): Info(Info), startTime(startTime), stopTime(startTime) {}
    DisplaySignalCharacter(const SignalInfo& Info, qint64 startTime, QString Remark): Info(Info), startTime(startTime), stopTime(startTime), Remark(Remark), isLegal(false) {}
};

enum MODEL_USER_VIEW
{
    SIGNAL_DETECT_TABLE,
    DISTURB_NOISE_TABLE,
    MAN_MADE_NOISE_TABLE
};

class SignalNoiseModel: public WBSignalDetectModel
{
    Q_OBJECT
public:
    explicit SignalNoiseModel(QObject* = nullptr);
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    bool setData(const QModelIndex&, const QVariant&, int = Qt::EditRole) override;
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int = Qt::DisplayRole) const override;

    void setUserViewType(MODEL_USER_VIEW);
    void setBandwidthThreshold(uint);
    void setFreqPointThreshold(uint);
    void setAmplThreshold(float);

    bool GenerateExcelSignalDetectTable();
    bool GenerateExcelDisturbNoiseTable(const CommonInfoSet&);

    void findPeakCyclically(Ipp32f*, int, unsigned long long, unsigned long long);
    bool ImportLegalFreqConf();
    bool ExportLegalFreqConf();
    void TriggerLegalFreqSet(bool); //控制进入合法频点设置阶段flag，正在修改合法频点属性时不更新界面元素，完成修改时更新map
    void TriggerRemarkSet(bool);
    void CleanUp(); //清理当前map中全部数据，开始重新计算
    MODEL_USER_VIEW m_eUserViewType = SIGNAL_DETECT_TABLE;

public slots:
    void UpdateData() override; //更新表格中数据，用户在选择当前信号是否合法时不进行界面更新

protected:
    std::vector<std::pair<QUuid, std::vector<QVariant>>> m_DisplayData;

private:
    std::map<SignalBaseChar, DisplaySignalCharacter> m_mapValidSignalCharacter;
    int m_iFullBandWidth;
    bool m_IsSettingLegalFreqFlag = false, m_IsSettingRemarkFlag = false;
    static constexpr const char* HEADER_LABEL_SIGNAL_DETECT[] = { "中心频率(MHz)", "电平(dBuV)", "带宽(MHz)", "起始时间", "结束时间", "占用带宽", "信号占用度", "合法信号" },
                        * HEADER_LABEL_DISTURB_NOISE[] = { "中心频率(MHz)", "大信号电平(dBuV)", "起始时间", "结束时间", "说明" };
    static constexpr auto LENGTH_SIGNAL_DETECT = sizeof(HEADER_LABEL_SIGNAL_DETECT) / sizeof(HEADER_LABEL_SIGNAL_DETECT[0]),
                        LENGTH_DISTURB_NOISE = sizeof(HEADER_LABEL_DISTURB_NOISE) / sizeof(HEADER_LABEL_DISTURB_NOISE[0]);
    static constexpr int LIST_LIMIT = 500;
};

#endif // SIGNALNOISEMODEL_H
