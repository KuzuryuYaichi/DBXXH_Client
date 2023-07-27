#ifndef ZCWIDGET_H
#define ZCWIDGET_H

#include <QDoubleSpinBox>

#include "ChartNB.h"
#include "TcpSocket.h"
#include "global.h"

class ZCWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ZCWidget(TcpSocket*, QWidget* = nullptr);
    void FFT_Trans(StructNBWaveZCResult*, int);

private:
    TcpSocket *m_socket;
    ChartNB *freqChart[ZC_NB_CHANNEL_NUM];
    QGridLayout *mainGridLayout;

    QDoubleSpinBox* rfCenterFreq;
    QComboBox* workMode;
    QSpinBox* rfDesc;
    QSpinBox* mfDesc;
    QPushButton* reListen;
};

#endif // ZCWIDGET_H
