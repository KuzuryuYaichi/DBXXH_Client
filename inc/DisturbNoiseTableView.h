#ifndef DISTURBNOISETABLEVIEW_H
#define DISTURBNOISETABLEVIEW_H

#include <QTableView>

class DisturbNoiseTableView: public QTableView
{
    Q_OBJECT
public:
    explicit DisturbNoiseTableView(QWidget *parent = nullptr);

    bool GenerateExcelTable(QString folderName);
protected:
    virtual ~DisturbNoiseTableView();
};

#endif // DISTURBNOISETABLEVIEW_H
