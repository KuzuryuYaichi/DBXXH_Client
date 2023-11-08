#include "ChannelWidget.h"

ChannelWidget::ChannelWidget(TcpSocket* socket, QWidget* parent): QWidget(parent), m_socket(socket)
{
    auto mainLayout = new QVBoxLayout(this);

    auto channelGroupBox = new QGroupBox(tr("Channel Visibility"));
    mainLayout->addWidget(channelGroupBox, 1);
    auto settingLayout = new QHBoxLayout(channelGroupBox);
    for (auto i = 1; i < ZC_NB_CHANNEL_NUMS; ++i)
    {
        settingLayout->addWidget(channelSec[i - 1] = new QCheckBox(tr("Channel %1").arg(i)));
        channelSec[i - 1]->setCheckState(Qt::Checked);
        connect(channelSec[i - 1], &QCheckBox::stateChanged, this, [this, i](int state) {
            chartNB[i]->setVisible(state);
        });
    }

    auto scrollArea = new QScrollArea(this);
    auto scrollWidget = new QWidget(this);
    auto scrollLayout = new QHBoxLayout(scrollWidget);
    for (int i = 0; i < ZC_NB_CHANNEL_NUMS; ++i)
    {
        connect(chartNB[i] = new ChartWidgetNB(tr("NB") + (i > 0? QString::number(i): ""), i), &ChartWidgetNB::ParamsChanged, this,
            [this, i] (unsigned long long freq, unsigned int bandwidth, unsigned int demodType, unsigned int cw, unsigned int RatePSK, char Truncate)
        {
            m_socket->nb_parameter_set(1, i, freq, bandwidth, demodType, cw, RatePSK, Truncate);
        });
        if (i > 0)
            scrollLayout->addWidget(chartNB[i]);
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

