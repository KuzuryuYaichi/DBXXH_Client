﻿#ifndef MAIN_WIDGET_H
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
    double Distance;
    double MaxFreq, MaxAmpl;
};

#endif // ChartWidget_H
