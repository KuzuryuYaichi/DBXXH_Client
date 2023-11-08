#ifndef MANMADENOISEMODEL_H
#define MANMADENOISEMODEL_H

#include "WBSignalDetectModel.h"
#include "SignalDetect/CommonInfoDialog.h"
#include <QLineF>

struct ManMadeNoiseInfo
{
    int CenterFreq;
    float Amp;
    qint64 startTime = 0;
    qint64 stopTime = 0;

    ManMadeNoiseInfo(int CenterFreq, float Amp, qint64 startTime): CenterFreq(CenterFreq), Amp(Amp), startTime(startTime) {}

    bool operator==(const ManMadeNoiseInfo& other) const
    {
//        if (CenterFreq != other.CenterFreq) //频点
//            return false;
//        if (startTime != other.startTime || stopTime != other.stopTime) //时间元素
//            return false;
        return CenterFreq == other.CenterFreq;
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
        if (amp < other.amp)
            return true;
        else if (amp == other.amp)
            return freqPointPos < other.freqPointPos;
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
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    bool setData(const QModelIndex&, const QVariant&, int = Qt::EditRole) override;
    QVariant data(const QModelIndex&, int = Qt::DisplayRole) const override;
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex&) const override;

    const std::map<int, std::map<int, double>>& lstTypicalFreq() const;
    void setLstTypicalFreq(const std::list<int>&);
    void findNoiseCharaAroundTypicalFreq(Ipp32f*, int, int, int);

    bool GenerateExcelManMadeNoiseTable(const CommonInfoSet&);
    bool GenerateWordManMadeNoiseChart(QAxObject*);

public slots:
    //更新表格中数据，用户在选择当前信号是否合法时不进行界面更新
    void UpdateData() override;
    void SetTime(bool) override;

protected:
    std::vector<std::vector<QVariant>> m_DisplayData;

private:
    void getManMadeNoiseAmpInEveryTestFreqSpanFromFullSpan(Ipp32f*, int, int, int);
    bool Intersects(const QLineF&, const QLineF&, QPointF&);
    //积累下来信号 key: SignalBaseChar ，头部节点为首次识别到该频点时的记录，需频点与带宽都满足区分阈值内才能算作有效信号
    //用于统计人为噪声（底噪）在各个频点时间内的表现形式 key: 典型频率点
    std::map<int, std::list<ManMadeNoiseInfo>> m_mapManMadeNoiseCharacter;
    ManMadeNoiseAnalyse m_ManMadNoiseAnalyse;
    qint64 m_iFindNoiseCharaTimeGap = 0;
    int m_Bandwidth = 1;

    static constexpr const char* HEADER_LABEL[] = { "典型频率点(MHz)", "测量频率(MHz)", "起始时间", "结束时间", "测量电平(dBuV)" };
    static constexpr auto LENGTH = sizeof(HEADER_LABEL) / sizeof(HEADER_LABEL[0]);
    static constexpr int LEVEL_LENGTH = 5;
    static constexpr double c[] = { 53.6, 67.2, 72.5, 76.8 };
    static constexpr double d[] = { 28.6, 27.7, 27.7, 27.7 };
    static constexpr int SAMPLE_POINTS = 7;
    static constexpr std::pair<int, double> AF[] =
        { {0.009, 5.91}, {1, 5.71}, {1.5, 5.51}, {3, 4.3}, {5, 5.37}, {7, 4.89}, {9, 1.29}, {12, 3.96}, {14, 0.72},
          {16, 1.67}, {18, 2.04}, {20, 2.81}, {22, 3.97}, {24, 4}, {26, 3.79}, {28, 3.93}, {30, 2.5} };
    static constexpr auto AF_LENGTH = sizeof(AF) / sizeof(AF[0]);
    static constexpr const char* POSITION_LEVEL[] = { "实例", "寂静乡村", "农村", "住宅", "城市" };
    double fa[LEVEL_LENGTH][SAMPLE_POINTS];
    void AddSeries(QAxObject*, const std::vector<std::pair<int, double>>&);
    void AddResult(QAxObject*, const std::vector<std::pair<int, double>>&);
};

#endif // MANMADENOISEMODEL_H
