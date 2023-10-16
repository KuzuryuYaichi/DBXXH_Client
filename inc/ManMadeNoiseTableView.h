#ifndef MANMADENOISETABLEVIEW_H
#define MANMADENOISETABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QAxObject>
#include <unordered_map>

#include "ManMadeNoiseModel.h"

class ManMadeNoiseTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ManMadeNoiseTableView(QWidget* = nullptr);
    ManMadeNoiseModel* m_pManMadeNoiseModel;
};

#endif // MANMADENOISETABLEVIEW_H
