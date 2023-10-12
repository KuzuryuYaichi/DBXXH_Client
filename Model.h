#ifndef MODEL_H
#define MODEL_H

#include <QMainWindow>
#include <QDateTime>
#include "ChannelWidget.h"
#include "MainWidget.h"
#include "TinyConfig.h"
#include "DataProcess.h"

class Model: public QMainWindow
{
    Q_OBJECT
public:
    Model(QWidget* = nullptr);
    ~Model();
    void showDataWB(const std::shared_ptr<unsigned char[]>&);
    void showDataNB(const std::shared_ptr<unsigned char[]>&);
    void showDataPulse(const std::shared_ptr<unsigned char[]>&);
    static QDateTime timeConvert(unsigned long long);
signals:
    void updatetime(unsigned long long);
    void sendDeviceStatus(QString);

private:
    TinyConfig* tinyConfig;
    ChannelWidget* m_channelWidget;
    MainWidget* m_mainWidget;
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
