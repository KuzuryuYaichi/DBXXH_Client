#ifndef ChartWidget_H
#define ChartWidget_H

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
#include "PointTableWidget.h"
#include "SideWidget.h"
#include "ChartNB.h"
#include "ChartWB.h"
#include "inc/WBSignalDetectWidget.h"

class ChartWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ChartWidget(TcpSocket*, ChartNB* chartNB, QWidget* = nullptr);
    void createSettings();

signals:
    void connectToServer(QString, quint16);

public:
    ChartNB* chartNB;
    ChartWB* chartWB;
    WBSignalDetectWidget* wBSignalDetectWidget;
    SideWidget* statusEdit;

private:
    TcpSocket *m_socket;
    QWidget *zcWidget;

    QLineEdit *ipEdit;
    QLineEdit *portEdit;
};

#endif // ChartWidget_H
