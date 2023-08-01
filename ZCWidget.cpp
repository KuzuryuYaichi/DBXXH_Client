#include "ZCWidget.h"

ZCWidget::ZCWidget(TcpSocket* socket, QWidget* parent): QWidget(parent), m_socket(socket)
{
    auto mainLayout = new QVBoxLayout(this);

    auto rfLayout = new QHBoxLayout;
    rfLayout->setAlignment(Qt::AlignLeft);
    rfLayout->addWidget(new QLabel(tr("复听:")));
    rfLayout->addWidget(reListen = new QPushButton(tr("选择文件")));
    mainLayout->addLayout(rfLayout, 1);

    auto scrollArea = new QScrollArea(this);
    auto scrollWidget = new QWidget(this);
    auto scrollLayout = new QHBoxLayout(scrollWidget);
    for (int i = 0; i < ZC_NB_PAGE_NUMS; ++i)
    {
        scrollLayout->addWidget(chartNB[i] = new ChartNB(tr("窄带") + QString::number(i + 1), tr("Freq(MHz)"), MIN_FREQ, MAX_FREQ, tr("Power(dBm)"), MIN_AMPL, MAX_AMPL));
    }
    for (int i = 0; i < ZC_NB_PAGE_NUMS; ++i)
    {
        for (int j = 0; j < ZC_NB_PAGE_NUMS; ++j)
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
    if (channel < 0 || channel >= ZC_NB_PAGE_NUMS)
        return;
    chartNB[channel]->replace(buf);
}

