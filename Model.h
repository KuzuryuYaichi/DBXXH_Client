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
    void showDataCX(unsigned char* const);
    void showDataZC(unsigned char* const, const QDateTime&);
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
    QStatusBar *statusBar;
    std::thread processThread;
    std::shared_ptr<TcpSocket> m_socket;
    bool isRunning = true;
    std::shared_ptr<FixedThreadPool> pool;
};

#endif // MODEL_H
