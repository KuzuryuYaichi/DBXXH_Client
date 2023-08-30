#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include <QDoubleSpinBox>

#include "TcpSocket.h"
#include "ChartWidgetNB.h"
#include "global.h"

class ChannelWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ChannelWidget(TcpSocket*, QWidget* = nullptr);
    void replace(unsigned char* const, int);
    ChartWidgetNB *chartNB[ZC_NB_CHANNEL_NUMS];

private:
    TcpSocket *m_socket;
    QGridLayout *mainGridLayout;
    QDoubleSpinBox* rfCenterFreq;
    QComboBox* workMode;
    QSpinBox* rfDesc;
    QSpinBox* mfDesc;
    QPushButton* reListen;
};

#endif // CHANNELWIDGET_H
