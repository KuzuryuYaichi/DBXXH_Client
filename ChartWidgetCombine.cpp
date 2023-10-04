#include "ChartWidgetCombine.h"

ChartWidgetCombine::ChartWidgetCombine(QWidget* parent): QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);

    m_updater = new QTimer;
    m_updater->setInterval(REFRESH_INTERVAL);
    m_updater->setSingleShot(true);
    connect(m_updater, &QTimer::timeout, this, [this] {
        ready = true;
    });
}
