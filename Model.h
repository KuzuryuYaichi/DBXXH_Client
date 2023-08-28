#ifndef MODEL_H
#define MODEL_H

#include <QMainWindow>
#include <QDateTime>
#include "ZCWidget.h"
#include "ChartWidget.h"
#include "TinyConfig.h"
#include "SqlWidget.h"
#include "DataProcess.h"

class Model: public QMainWindow
{
    Q_OBJECT
public:
    Model(QWidget *parent = nullptr);
    ~Model();
    void showDataCX(unsigned char* const);
    void showDataZC(unsigned char* const, const QDateTime&);
    static QDateTime timeConvert(unsigned long long);
signals:
    void updatetime(unsigned long long);
    void sendDeviceStatus(QString);

private:
    TinyConfig* tinyConfig;
    ZCWidget* m_zcWidget;
    ChartWidget* m_cxWidget;
    SqlWidget* m_sqlWidget;
    QTabWidget* m_tabWidget;
    QString m_statusList;
    QStatusBar *statusBar;
    std::unique_ptr<DataProcess> dataProcess;
    std::thread processThread;
    std::shared_ptr<TcpSocket> socket;
    QTimer* statusTimer;
    bool readyTime = true;
    bool Running = true;
};

#endif // MODEL_H
