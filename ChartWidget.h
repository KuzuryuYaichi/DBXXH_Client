#ifndef ChartWidget_H
#define ChartWidget_H

#include <QWidget>
#include <QChartView>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTextEdit>
#include <QLayout>
#include <QTableWidget>

#include "global.h"
#include "TcpSocket.h"
#include "PointTableWidget.h"
#include "SideWidget.h"
#include "ChartNB.h"
#include "ChartWB.h"
#include "TableSignals.h"
#include "TableInterference.h"
#include "TableNoise.h"

class ChartWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ChartWidget(TcpSocket*, QWidget* = nullptr);
    void createSettings();

signals:
    void connectToServer(QString, quint16);

public:
    ChartNB *chartNB;
    ChartWB *chartWB;
    TableSignals* tableSignals;
    TableInterference* tableInterference;
    TableNoise* tableNoise;
    PointTableView* freqListTable;
    SideWidget* statusEdit;
    double threshold = 0;

private:
    TcpSocket *m_socket;

    QWidget *zcWidget;

    QPushButton *hideBtn;
    QPushButton *continueBtn;
    QPushButton *clrBtn;

    QComboBox *freqResBox;
    QComboBox *smoothBox;
    QComboBox *gainModeBox;

    QLineEdit *ipEdit;
    QLineEdit *portEdit;

    QDoubleSpinBox *gainEdit;
    QLineEdit *LmValEdit;
    QComboBox *rcvModeBox;
};

#endif // ChartWidget_H
