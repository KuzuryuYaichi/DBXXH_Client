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
    void createSettings();

signals:
    void connectToServer(QString, quint16);

public:
    ChartWidgetNB* chartNB;
    ChartWidgetWB* chartWB;
    WBSignalDetectWidget* wBSignalDetectWidget;

private:
    TcpSocket *m_socket;
    QWidget *zcWidget;

    QLineEdit *ipEdit;
    QLineEdit *portEdit;
    QLabel* MarkerLbl[MARKER_NUM], *MeasureLbl, *TrackLbl;
    QTimer* m_updater;

    std::vector<std::pair<bool, double>> MarkAmpl;
    double Distance;
    QString MaxPoint;
};

#endif // ChartWidget_H
