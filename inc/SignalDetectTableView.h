#ifndef SIGNALDETECTTABLEVIEW_H
#define SIGNALDETECTTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QItemDelegate>
#include <QComboBox>
#include <QAxObject>

class CheckBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit CheckBoxDelegate(QObject *parent = nullptr);
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class SignalDetectTableView : public QTableView
{
    Q_OBJECT
public:
    explicit SignalDetectTableView(QWidget* = nullptr);
    bool GenerateExcelTable(QString);
    void GenerateSignalDetectTable(QAxObject*);
};

#endif // SIGNALDETECTTABLEVIEW_H
