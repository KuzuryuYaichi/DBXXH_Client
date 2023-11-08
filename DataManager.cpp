#include "DataManager.h"
#include "global.h"

extern PARAMETER_SET g_parameter_set;

DataManager::DataManager(QObject* parent): QObject(parent)
{
    g_parameter_set.tinyConfig = TinyConfig("config.ini");
}

bool DataManager::load()
{
    translate(":/client_cn.qm");
#if defined(Q_OS_MAC)
    translate(AppPath + "/translations/qt_zh_CN.qm");
#else
    translate(QApplication::applicationDirPath() + "/client_cn.qm");
#endif
    return true;
}

bool DataManager::translate(const QString& filename)
{
    auto t = new QTranslator(this);
    if (t->load(filename))
    {
        qApp->installTranslator(t);
        Translators.emplace_back(t);
        return true;
    }
    delete t;
    return false;
}

void DataManager::eliminate()
{
    for (auto& t: Translators)
    {
        qApp->removeTranslator(t);
        delete t;
    }
    Translators.clear();
}

DataManager::~DataManager()
{
    eliminate();
}
