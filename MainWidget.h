#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTimer>

#include "TcpSocket.h"
#include "ChartWidgetNB.h"
#include "ChartWidgetWB.h"
#include "inc/WBSignalDetectWidget.h"

class MainWidget: public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(TcpSocket*, ChartWidgetNB* chartNB, QWidget* = nullptr);

signals:
    void connectToServer(QString, quint16);

public:
    ChartWidgetNB* chartNB;
    ChartWidgetWB* chartWB;
    WBSignalDetectWidget* wbSignalDetectWidget;

private:
    TcpSocket *m_socket;
    QWidget *zcWidget;

    QLineEdit *ipEdit;
    QLineEdit *portEdit;
    QLabel* MarkerFreqLbl[MARKER_NUM], * MarkerAmplLbl[MARKER_NUM], *MeasureLbl, *MaxFreqLbl, *MaxAmplLbl;
    QTimer* m_updater;
    QPushButton* saveBtn;

    std::vector<std::tuple<bool, double, double>> MarkData;
    double Distance = 0;
    double MaxFreq = MIN_FREQ, MaxAmpl = MAX_AMPL;
};

#endif // ChartWidget_H
