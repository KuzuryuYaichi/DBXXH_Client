#include "ChannelWidget.h"

ChannelWidget::ChannelWidget(TcpSocket* socket, QWidget* parent): QWidget(parent), m_socket(socket)
{
    auto mainLayout = new QVBoxLayout(this);

    auto scrollArea = new QScrollArea(this);
    auto scrollWidget = new QWidget(this);
    auto scrollLayout = new QHBoxLayout(scrollWidget);
    for (int i = 0; i < ZC_NB_CHANNEL_NUMS; ++i)
    {
        chartNB[i] = new ChartWidgetNB(tr("NB") + (i > 0? QString::number(i): ""), i);
        connect(chartNB[i], &ChartWidgetNB::ParamsChanged, this, [this, i] (unsigned long long freq, unsigned int bandwidth, unsigned int demodType, unsigned int cw) {
            m_socket->nb_parameter_set(1, i, freq, bandwidth, demodType, cw);
        });
        if (i > 0)
        {
            scrollLayout->addWidget(chartNB[i]);
        }
    }
    for (int i = 0; i < ZC_NB_CHANNEL_NUMS; ++i)
    {
        for (int j = 0; j < ZC_NB_CHANNEL_NUMS; ++j)
        {
            connect(chartNB[i], &ChartWidgetNB::triggerListening, chartNB[j], &ChartWidgetNB::changedListening);
        }
    }
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea, 25);

    AudioThread = new ThreadAudio(this);
}

void ChannelWidget::replace(const std::shared_ptr<unsigned char[]>& data, int channel)
{
    if (channel < 0 || channel >= ZC_NB_CHANNEL_NUMS)
        return;
    chartNB[channel]->replace(data);
    if (chartNB[channel]->playing)
        AudioThread->execute(data);
}

