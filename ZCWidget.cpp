#include "ZCWidget.h"

ZCWidget::ZCWidget(TcpSocket* socket, QWidget* parent): QWidget(parent), m_socket(socket)
{
    auto mainLayout = new QVBoxLayout(this);

    auto rfLayout = new QHBoxLayout;
    rfLayout->setAlignment(Qt::AlignLeft);
    rfLayout->addWidget(new QLabel(tr("复听:")));
    rfLayout->addWidget(reListen = new QPushButton(tr("选择文件")));
//    mainLayout->addLayout(rfLayout, 1);

    auto scrollArea = new QScrollArea(this);
    auto scrollWidget = new QWidget(this);
    auto scrollLayout = new QHBoxLayout(scrollWidget);
    for (int i = 0; i < ZC_NB_CHANNEL_NUMS; ++i)
    {
        chartNB[i] = new ChartNB(tr("窄带") + (i > 0? QString::number(i): ""));
        if (i > 0)
        {
            scrollLayout->addWidget(chartNB[i]);
            connect(chartNB[i], &ChartNB::ParamsChanged, this, [this, i] (unsigned long long freq, unsigned int bandwidth, unsigned int demodType) {
                m_socket->nb_channel(1, i, freq, bandwidth, demodType);
            });
        }
    }
    for (int i = 0; i < ZC_NB_CHANNEL_NUMS; ++i)
    {
        for (int j = 0; j < ZC_NB_CHANNEL_NUMS; ++j)
        {
            if (i != j)
                connect(chartNB[i], &ChartNB::triggerListening, chartNB[j], &ChartNB::changedListening);
        }
    }
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea, 25);
}

void ZCWidget::replace(unsigned char* const buf, int channel)
{
    if (channel < 0 || channel >= ZC_NB_CHANNEL_NUMS)
        return;
    chartNB[channel]->replace(buf);
}

