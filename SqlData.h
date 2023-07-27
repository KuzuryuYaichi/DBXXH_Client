#ifndef SQLDATA_H
#define SQLDATA_H

#include <QSql>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QTableView>
#include <QDateTime>

class SqlData
{
public:
    SqlData(QString, QTableView* = nullptr);
    ~SqlData();

private:
    QSqlDatabase dbconn;
    QSqlQueryModel* model;

public:
    void Insert(const QVector<std::tuple<double, double, int>>&, const QDateTime&);
    long long QueryCount(double, double, int, QDateTime, QDateTime);
    void Query(double, double, int, QDateTime, QDateTime, int = 0);

private:
    void CreateDataBase(QString);

    static constexpr char HOST_NAME[] = "localhost";
    QString DB_NAME;
    static constexpr char USER_NAME[] = "root";
    static constexpr char PASSWORD[] = "123456";
};

#endif
