#ifndef DISTURBNOISETABLEVIEW_H
#define DISTURBNOISETABLEVIEW_H

#include <QTableView>
#include <QAxObject>

class DisturbNoiseTableView: public QTableView
{
    Q_OBJECT
public:
    explicit DisturbNoiseTableView(QWidget* = nullptr);
};

#endif // DISTURBNOISETABLEVIEW_H
