#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTextEdit>
#include <QLayout>
#include <QTableWidget>

#include "TcpSocket.h"
#include "SideWidget.h"
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
    SideWidget* statusEdit;

private:
    TcpSocket *m_socket;
    QWidget *zcWidget;

    QLineEdit *ipEdit;
    QLineEdit *portEdit;
    QLabel* Marker[MARKER_NUM];
};

#endif // ChartWidget_H
