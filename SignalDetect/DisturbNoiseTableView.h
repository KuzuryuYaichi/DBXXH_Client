#ifndef DISTURBNOISETABLEVIEW_H
#define DISTURBNOISETABLEVIEW_H

#include <QTableView>
#include <QItemDelegate>
#include <QAxObject>

class LineEditDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit LineEditDelegate(QObject* = nullptr);
    QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget*, const QModelIndex&) const override;
    void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const override;
    void updateEditorGeometry(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
};

class DisturbNoiseTableView: public QTableView
{
    Q_OBJECT
public:
    explicit DisturbNoiseTableView(QWidget* = nullptr);
};

#endif // DISTURBNOISETABLEVIEW_H
