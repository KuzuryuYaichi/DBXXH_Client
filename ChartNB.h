#ifndef CHARTNB_H
#define CHARTNB_H

#include "CombineWidget.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>

class ChartNB: public CombineWidget
{
    Q_OBJECT
public:
    ChartNB(QString, QWidget* = nullptr);
    void replace(unsigned char* const);
signals:
    void triggerListening(bool);
    void FreqBandwidthChanged(unsigned long long, unsigned int, unsigned int);
public slots:
    void changedListening(bool);
    void changedRecording(bool);

private:
    enum SHOW_MODE
    {
        DDC_MODE = 0,
        FFT_MODE
    };
    QDoubleSpinBox* freqEdit;
    QComboBox* bandBox;
    QComboBox* demodBox;
    QComboBox* showBox;
    QPushButton* playBtn;
    bool playing = false;
    bool recording = false;
    static constexpr auto DDC_LEN = 2048;
    static constexpr auto AMPL_OFFSET = -206;
};

#endif // CHARTNB_H
