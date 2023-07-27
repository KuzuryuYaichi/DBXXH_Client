#include "TinyConfig.h"

#include <QRegularExpression>
#include <QFileInfo>

QString TinyConfig::Get_DataIP()
{
    return GetConfigData(Data_Area, "DataIP").toString();
}

unsigned short TinyConfig::Get_DataPort()
{
    return GetConfigData(Data_Area, "DataPort").toInt();
}

void TinyConfig::Set_Data(QString& DataIP, int& DataPort)
{
    SetConfigData(Data_Area, "DataIP", DataIP);
    SetConfigData(Data_Area, "DataPort", DataPort);
}

TinyConfig::TinyConfig()
{

}

TinyConfig::TinyConfig(const QString& fileName)
{
    auto configFileName = QApplication::applicationDirPath() + "/" + fileName;
    m_psetting = new QSettings(configFileName, QSettings::IniFormat);

    QFileInfo fileInfo(configFileName);
    if (!fileInfo.exists())
    {
        SetConfigData(Data_Area, "DataIP", DEFAULT_SERVER_DATA_IP);
        SetConfigData(Data_Area, "DataPort", DEFAULT_SERVER_DATA_PORT);
    }

    QString DataIP = GetConfigData(Data_Area, "DataIP").toString();
    int DataPort = GetConfigData(Data_Area, "DataPort").toInt();
    if (isIpAddr(DataIP) && DataPort > 0 && DataPort < 65536) {
        this->DataIP = DataIP;
        this->DataPort = DataPort;
    }
}

void TinyConfig::SetConfigData(QString qstrnodename, QString qstrkeyname, QVariant qvarvalue)
{
    if (m_psetting) {
        m_psetting->setValue(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname), qvarvalue);
    }
}

QVariant TinyConfig::GetConfigData(QString qstrnodename, QString qstrkeyname)
{
    QVariant qvar = -1;
    if (m_psetting) {
        qvar = m_psetting->value(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname));
    }
    return qvar;
}

bool TinyConfig::isIpAddr(const QString& ip)
{
    return QRegularExpression("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)").match(ip).hasMatch();
}
