#include "SqlData.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QDebug>
#include <QApplication>

SqlData::SqlData(QString connectionName, QTableView* tableView): DB_NAME(QApplication::applicationDirPath() + "/NACX.db")
{
    CreateDataBase(connectionName);
    if (tableView == nullptr)
        return;
    model = new QSqlQueryModel;
    tableView->setModel(model);
}

SqlData::~SqlData()
{
    if (dbconn.isOpen())
        dbconn.close();
}

void SqlData::CreateDataBase(QString connectionName)
{
    dbconn = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    dbconn.setHostName(HOST_NAME);
    dbconn.setDatabaseName(DB_NAME);
    if(dbconn.open())
    {
        qDebug() << connectionName << "Open Succeed";
    }
    QSqlQuery query(dbconn);
    auto res = query.exec("CREATE TABLE IF NOT EXISTS SignalCollection ( \
                          `index` integer NOT NULL PRIMARY KEY AUTOINCREMENT, \
                          freq real, \
                          direction real, \
                          confidence integer, \
                          time DATETIME \
                        );"
                        );
    if (!res)
        qDebug() << "Create Table Error: " << dbconn.lastError().text();
}

void SqlData::Insert(const QVector<std::tuple<double, double, int>>& data, const QDateTime& time)
{
    if (data.size() == 0)
        return;
    auto time_t = time.toSecsSinceEpoch();
    QString sqlStr("INSERT INTO SignalCollection (freq, direction, confidence, time) VALUES (%1, %2, %3, %4);");
    auto size = data.size();
    dbconn.transaction();
    for (int i = 0; i < size; ++i)
    {
        auto& d = data[i];
        dbconn.exec(sqlStr.arg(std::get<0>(d)).arg(std::get<1>(d)).arg(std::get<2>(d)).arg(time_t));
    }
    dbconn.commit();
}

long long SqlData::QueryCount(double startFreq, double endFreq, int confindece_gate, QDateTime startTime, QDateTime endTime)
{
    QString sqlStr("SELECT COUNT(*) FROM SignalCollection WHERE (freq >= %1 AND freq <= %2 AND confidence > %3 AND time >= %4 AND time <= %5);");
    sqlStr = sqlStr.arg(startFreq).arg(endFreq).arg(confindece_gate).arg(startTime.toSecsSinceEpoch()).arg(endTime.toSecsSinceEpoch());
    QSqlQuery query(dbconn);
    query.exec(sqlStr);
    if (!query.next())
        return -1;
    bool res;
    auto count = query.value(0).toLongLong(&res);
    return res? count: -1;
}

void SqlData::Query(double startFreq, double endFreq, int confindece_gate, QDateTime startTime, QDateTime endTime, int Offset)
{
    QString sqlStr("SELECT freq, direction, confidence, DATETIME(time, 'unixepoch', 'localtime') AS time FROM signalcollection WHERE (freq >= %1 AND freq <= %2 AND confidence > %3 AND time >= %4 AND time <= %5) LIMIT 200 OFFSET %6;");
    sqlStr = sqlStr.arg(startFreq).arg(endFreq).arg(confindece_gate).arg(startTime.toSecsSinceEpoch()).arg(endTime.toSecsSinceEpoch()).arg(Offset);
    model->setQuery(sqlStr, dbconn);
}
