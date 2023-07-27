#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QTranslator>
#include <QApplication>

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
    QList<QTranslator*> Translators;
};

#endif // DATAMANAGER_H
