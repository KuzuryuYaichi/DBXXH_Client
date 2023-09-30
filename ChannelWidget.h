#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include <QDoubleSpinBox>

#include "TcpSocket.h"
#include "ChartWidgetNB.h"
#include "ThreadAudio.h"
#include "global.h"

class ChannelWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ChannelWidget(TcpSocket*, QWidget* = nullptr);
    void replace(std::shared_ptr<unsigned char[]>, int);
    ChartWidgetNB *chartNB[ZC_NB_CHANNEL_NUMS];

private:
    ThreadAudio* AudioThread;
    TcpSocket *m_socket;
    QGridLayout *mainGridLayout;
    QDoubleSpinBox* rfCenterFreq;
    QComboBox* workMode;
    QSpinBox* rfDesc;
    QSpinBox* mfDesc;
    QPushButton* reListen;
};

#endif // CHANNELWIDGET_H
