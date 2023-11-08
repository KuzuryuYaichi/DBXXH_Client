#include "TinyConfig.h"

#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

QString TinyConfig::Get_DataIP()
{
    return DataIP;
}

unsigned short TinyConfig::Get_DataPort()
{
    return DataPort;
}

QString TinyConfig::Get_StoragePath()
{
    return StoragePath;
}

unsigned long long TinyConfig::Get_StorageThreshold()
{
    return StorageThreshold;
}

TinyConfig::TinyConfig(const QString& fileName)
{
    m_psetting = new QSettings(fileName, QSettings::IniFormat);
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists())
    {
        SetConfigData(Data_Area, "DataIP", DEFAULT_SERVER_DATA_IP);
        SetConfigData(Data_Area, "DataPort", DEFAULT_SERVER_DATA_PORT);
        SetConfigData(Data_Area, "StoragePath", DEFAULT_STORAGE_PATH);
        SetConfigData(Data_Area, "StorageThreshold", DEFAULT_STORAGE_THRESHOLD);
    }
    bool toNumber;
    auto DataIP = GetConfigData(Data_Area, "DataIP").toString();
    short DataPort = GetConfigData(Data_Area, "DataPort").toInt(&toNumber);
    if (!toNumber || !isIpAddr(DataIP) || (DataPort < 0 && DataPort > SHRT_MAX))
    {
        QMessageBox::warning(nullptr, "Config Error", "Invalid Network Format!");
        throw std::exception("Invalid Network Format");
    }
    this->DataIP = DataIP;
    this->DataPort = DataPort;
    QDir storage(StoragePath);
    if (!storage.exists())
        storage.mkdir(StoragePath);
    auto StoragePath = GetConfigData(Data_Area, "StoragePath").toString();
    auto StorageThreshold = GetConfigData(Data_Area, "StorageThreshold").toULongLong(&toNumber);
    if (!toNumber)
    {
        QMessageBox::warning(nullptr, "Config Error", "Invalid Storage Format!");
        throw std::exception("Invalid Storage Format");
    }
    this->StoragePath = StoragePath;
    this->StorageThreshold = StorageThreshold;
}

void TinyConfig::SetConfigData(QString qstrnodename, QString qstrkeyname, QVariant qvarvalue)
{
    m_psetting->setValue(QString("/%1/%2").arg(qstrnodename, qstrkeyname), qvarvalue);
}

QVariant TinyConfig::GetConfigData(QString qstrnodename, QString qstrkeyname)
{
    return m_psetting->value(QString("/%1/%2").arg(qstrnodename, qstrkeyname));
}

bool TinyConfig::isIpAddr(const QString& ip)
{
    static QRegularExpression IP_Regular("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    return IP_Regular.match(ip).hasMatch();
}
