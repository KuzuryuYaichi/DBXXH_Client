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
    ChartNB(QString, QString, int, int, QString, int, int, QWidget* = nullptr);
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
    QDoubleSpinBox* freqEdit;
    QComboBox* bandBox;
    QComboBox* showBox;
    QPushButton* playBtn;
    bool playing = false;
};

#endif // CHARTNB_H
