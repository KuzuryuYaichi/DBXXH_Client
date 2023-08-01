#ifndef CHARTNB_H
#define CHARTNB_H

#include "CombineWidget.h"
#include "ChartViewWave.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>

class ChartNB: public CombineWidget
{
    Q_OBJECT
public:
    ChartNB(QString, QString, int, int, QString, int, int, QWidget* = nullptr);
    void replace(unsigned char* const);
signals:
    void triggerListening(bool);
public slots:
    void changedListening(bool);

private:
    enum SHOW_MODE
    {
        DDC_MODE = 0,
        FFT_MODE
    };
    ChartViewWave* chartWave;
    QDoubleSpinBox* freqEdit;
    QComboBox* bandBox;
    QComboBox* showBox;
    QPushButton* playBtn;
    bool playing = false;
    static constexpr auto DDC_LEN = 2048;
    static constexpr auto AMPL_OFFSET = -206;
};

#endif // CHARTNB_H
