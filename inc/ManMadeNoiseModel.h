#ifndef MANMADENOISEMODEL_H
#define MANMADENOISEMODEL_H

#include "WBSignalDetectModel.h"

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
    std::map<int, std::map<int, double>> m_mapStoreAmpValueToGetManMadeNoiseValue;  //记录当前典型频率点对应的10个测量频点，包含每次分析所得的电平幅值，每次都与前一次算均值
};

class ManMadeNoiseModel: public WBSignalDetectModel
{
    Q_OBJECT
public:
    explicit ManMadeNoiseModel(QObject* = nullptr);
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex&) const override;

    const std::map<int, std::map<int, double>>& lstTypicalFreq() const;
    void setLstTypicalFreq(const std::list<int>&);
    std::map<int, int> mapExistTypicalFreqNoiseRecordAmount() const;
    void findNoiseCharaAroundTypicalFreq(Ipp32f*, int, int, int);

    bool GenerateExcelManMadeNoiseTable(const CommonInfoSet&);
    bool GenerateWordManMadeNoiseTable(QAxObject*);
    bool GenerateWordManMadeNoiseChart(QAxObject*);

public slots:
    //更新表格中数据，用户在选择当前信号是否合法时不进行界面更新
    void UpdateData() override;
//    void SetStartTime();
//    void SetStopTime();

private:
    void getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(Ipp32f*, int, int, int);
    //积累下来信号 key: SignalBaseChar ，头部节点为首次识别到该频点时的记录，需频点与带宽都满足区分阈值内才能算作有效信号
    //用于统计人为噪声（底噪）在各个频点时间内的表现形式 key: 典型频率点
    std::map<int, std::list<ManMadeNoiseInfo>> m_mapManMadeNoiseCharacter;
    ManMadeNoiseAnalyse m_ManMadNoiseAnalyse;
    qint64 m_iFindNoiseCharaTimeGap = 0;

    static constexpr const char* HEADER_LABEL[] = { "典型频率点(MHz)", "测量频率(MHz)", "起始时间", "结束时间", "测量电平(dBuV)" };
    static constexpr auto LENGTH = sizeof(HEADER_LABEL) / sizeof(HEADER_LABEL[0]);

private:
    static constexpr int LEVEL_LENGTH = 5;
    static constexpr double c[] = { 53.6, 67.2, 72.5, 76.8 };
    static constexpr double d[] = { 28.6, 27.7, 27.7, 27.7 };
    static constexpr double SAMPLE_FREQ[] = { 2, 2.5, 5, 10, 15, 20, 25 };
    static constexpr int SAMPLE_POINTS = sizeof(SAMPLE_FREQ) / sizeof(SAMPLE_FREQ[0]);
    static constexpr const char* POSITION_LEVEL[] = { "实例", "寂静", "乡村", "郊区", "城市"};
    static std::array<std::array<double, SAMPLE_POINTS>, LEVEL_LENGTH> F_AM()
    {
        std::array<std::array<double, SAMPLE_POINTS>, LEVEL_LENGTH> f_am;
        for (auto point = 1; point < LEVEL_LENGTH; ++point)
        {
            for (auto level = 0; level < SAMPLE_POINTS; ++level)
            {
                f_am[point][level] = c[point - 1] - d[point - 1] * std::log10(SAMPLE_FREQ[level]);
            }
        }
        return f_am;
    }
    std::array<std::array<double, SAMPLE_POINTS>, LEVEL_LENGTH> f_am;
    void AddSeries(QAxObject*, const std::vector<double>&);
    void AddResult(QAxObject* range);
};

#endif // MANMADENOISEMODEL_H
