#ifndef MODEL_H
#define MODEL_H

#include <QMainWindow>
#include <QDateTime>
#include "ZCWidget.h"
#include "ChartWidget.h"
#include "TinyConfig.h"
#include "FixedThreadPool.hpp"
#include "SqlWidget.h"

class Model: public QMainWindow
{
    Q_OBJECT
public:
    Model(QWidget *parent = nullptr);
    ~Model();
    void showDataCX(char* const);
    void showDataZC(char* const, const QDateTime&);
    static QDateTime timeConvert(unsigned long long);
signals:
    void updatetime(QString);
    void sendDeviceStatus(QString);

private:
    TinyConfig* tinyConfig;
    ZCWidget* m_zcWidget;
    ChartWidget* m_cxWidget;
    SqlWidget* m_sqlWidget;
    QTabWidget* m_tabWidget;
    QString m_statusList;

    unsigned long long start_freq_Hz;
    unsigned long long stop_freq_Hz;

    QStatusBar *statusBar;
    TcpSocket *m_socket;

    std::thread processThread;
    bool isRunning = true;
    threadsafe_queue<std::shared_ptr<char[]>> spsc_queue;
    std::shared_ptr<FixedThreadPool> pool;
};

#endif // MODEL_H
