#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QTranslator>
#include <list>

class DataManager: public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = nullptr);
    ~DataManager();
    bool load();
    bool translate(const QString &filename);
    void eliminate();

private:
    std::list<QTranslator*> Translators;
};

#endif // DATAMANAGER_H
