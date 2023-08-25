#ifndef ZCWIDGET_H
#define ZCWIDGET_H

#include <QDoubleSpinBox>

#include "TcpSocket.h"
#include "ChartNB.h"
#include "global.h"

class ZCWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ZCWidget(TcpSocket*, QWidget* = nullptr);
    void replace(unsigned char* const, int);
    ChartNB *chartNB[ZC_NB_CHANNEL_NUMS];

private:
    TcpSocket *m_socket;
    QGridLayout *mainGridLayout;
    QDoubleSpinBox* rfCenterFreq;
    QComboBox* workMode;
    QSpinBox* rfDesc;
    QSpinBox* mfDesc;
    QPushButton* reListen;
};

#endif // ZCWIDGET_H
