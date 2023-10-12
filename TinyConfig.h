#ifndef _TINY_CONFIG_H
#define _TINY_CONFIG_H

#include <QApplication>
#include <QSettings>

class TinyConfig
{
public:
    TinyConfig() = default;
    TinyConfig(const TinyConfig&) = default;
    TinyConfig& operator=(const TinyConfig&) = default;
    TinyConfig(const QString&);
    QString Get_DataIP();
    unsigned short Get_DataPort();
    QString Get_StoragePath();
    unsigned long long Get_StorageThreshold();

private:
    static constexpr const char DEFAULT_SERVER_DATA_IP[] = "127.0.0.1";
    static constexpr int DEFAULT_SERVER_DATA_PORT = 5021;
    static constexpr const char DEFAULT_STORAGE_PATH[] = "C:";
    static constexpr unsigned long long DEFAULT_STORAGE_THRESHOLD = 1e9;
    static constexpr const char Data_Area[] = "Data";

    QSettings* m_psetting;
    QString DataIP = DEFAULT_SERVER_DATA_IP;
    unsigned short DataPort = DEFAULT_SERVER_DATA_PORT;
    QString StoragePath = DEFAULT_STORAGE_PATH;
    unsigned long long StorageThreshold = DEFAULT_STORAGE_THRESHOLD;
    QVariant GetConfigData(QString, QString);
    void SetConfigData(QString, QString, QVariant);
    bool isIpAddr(const QString&);
};

#endif
