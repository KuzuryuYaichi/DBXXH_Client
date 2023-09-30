#include "ChannelWidget.h"

ChannelWidget::ChannelWidget(TcpSocket* socket, QWidget* parent): QWidget(parent), m_socket(socket)
{
    auto mainLayout = new QVBoxLayout(this);

    auto rfLayout = new QHBoxLayout;
    rfLayout->setAlignment(Qt::AlignLeft);
    rfLayout->addWidget(new QLabel(tr("ReListen:")));
    rfLayout->addWidget(reListen = new QPushButton(tr("Select File")));
//    mainLayout->addLayout(rfLayout, 1);

    auto scrollArea = new QScrollArea(this);
    auto scrollWidget = new QWidget(this);
    auto scrollLayout = new QHBoxLayout(scrollWidget);
    for (int i = 0; i < ZC_NB_CHANNEL_NUMS; ++i)
    {
        chartNB[i] = new ChartWidgetNB(tr("NB") + (i > 0? QString::number(i): ""), i);
        connect(chartNB[i], &ChartWidgetNB::ParamsChanged, this, [this, i] (unsigned long long freq, unsigned int bandwidth, unsigned int demodType, unsigned int cwOffset) {
            m_socket->nb_parameter_set(1, i, freq, bandwidth, demodType, cwOffset);
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

    AudioThread = new ThreadAudio;
    AudioThread->start();
}

void ChannelWidget::replace(std::shared_ptr<unsigned char[]> data, int channel)
{
    if (channel < 0 || channel >= ZC_NB_CHANNEL_NUMS)
        return;
    chartNB[channel]->replace(data.get());
    if (chartNB[channel]->playing)
        AudioThread->execute(data);
}

