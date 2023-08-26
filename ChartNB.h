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
    void changedRecording();

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
    QPushButton* recordBtn;
    bool playing = false;
    bool recording = false;
};

#endif // CHARTNB_H
