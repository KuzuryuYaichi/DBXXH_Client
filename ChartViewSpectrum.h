#ifndef CHARTVIEWSPECTRUM_H
#define CHARTVIEWSPECTRUM_H

#include "ChartViewCustom.h"

class ChartViewSpectrum: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewSpectrum(QString, QString, int, int, QString, int, int, QWidget* = nullptr);
    void replace(char* const buf);

private:
    void UpdateRuler(QMouseEvent *);
    void UpdateTracer(QMouseEvent *);
    QCPItemTracer* tracer;
    QTimer* m_updater;
    bool isPress = false;
    static constexpr int TIME_INTERVAL = 100;

signals:
    void thresholdEnterPressedSignal(double);
};

#endif // CHARTVIEWSPECTRUM_H
