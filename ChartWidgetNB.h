#ifndef CHARTWIDGETNB_H
#define CHARTWIDGETNB_H

#include "ChartWidgetCombine.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QFile>

#include "ChartViewWave.h"
#include "ChartViewSpectrumNB.h"
#include "ChartViewScatter.h"
#include "fftw-3.3.5-dll64/fftw3.h"

template<int N>
constexpr auto HanningWindow()
{
    std::array<double, N> WINDOW;
    for (int i = 0; i < N; i++)
    {
        WINDOW[i] = 0.5 * (1 - std::cos(2 * std::numbers::pi * (i + 1) / (N + 1)));
    }
    return WINDOW;
}

class ChartWidgetNB: public ChartWidgetCombine
{
    Q_OBJECT
public:
    ChartWidgetNB(QString, int, QWidget* = nullptr);
    ~ChartWidgetNB();
    virtual void ChangeMode(int) override;
    virtual void replace(const std::shared_ptr<unsigned char[]>&) override;
    void ChangeFreq(double);
    ChartViewWave* chartWave;
    ChartViewSpectrumNB* chartSpectrum;
    ChartViewScatter* chartScatter;
    bool playing = false;

signals:
    void triggerDepthAM(unsigned short, unsigned short);
    void triggerListening(int, bool);
    void ParamsChanged(unsigned long long, unsigned int, unsigned int, unsigned int, unsigned int, char);
public slots:
    void changedListening(int, bool);
    void changedRecording();

protected:
    void FFT(unsigned char*);
    bool TestRecordThreshold();
    void WriteFile(char*, int);
    void RemoveFile();
    bool CheckStorage();
    void Record(unsigned char* const);
    void ParamsChange();
    void ChangeDigitDemodRate();

    fftw_complex* inR, * outR;
    fftw_plan planR;
    std::unique_ptr<unsigned char[]> AmplData;
    QLabel* LblFSK, *LblDQPSK, *LblDepthAM, *DepthAM, *LblMute, *LblCW;
    QComboBox* demodBox, *RateBoxFSK, *RateBoxDQPSK;
    QPushButton* playBtn, *showWaveBtn;
    QPushButton* recordBtn;
    QSpinBox* cwEdit;
    QSlider* TruncateSlider;
    bool recording = false;
    QFile file;
    std::mutex fileLock;
    int index;
    double RecordThreshold = MAX_AMPL;
    bool showWave = true;
    QCheckBox* Tmp_Chk;

    enum SHOW_MODE
    {
        WAVE_MODE,
        SPECTRUM_MODE,
        WATERFALL_MODE,
        SCATTER_MODE
    };

    enum DEMOD_TYPE
    {
        IQ,
        AM,
        FM,
        PM,
        USB,
        LSB,
        ISB,
        CW,
        FSK,
        PSK
    };
private:
    static constexpr int REFRESH_INTERVAL = 150;
    bool ready = true;
};

#endif // CHARTWIDGETNB_H
