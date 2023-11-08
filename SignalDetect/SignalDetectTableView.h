#ifndef SIGNALDETECTTABLEVIEW_H
#define SIGNALDETECTTABLEVIEW_H

#include <QWidget>
#include <QCheckBox>
#include <QTableView>
#include <QItemDelegate>
#include <QComboBox>
#include <QAxObject>

class CheckBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit CheckBoxDelegate(QObject* = nullptr);
    QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget*, const QModelIndex&) const override;
    void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const override;
    void updateEditorGeometry(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
};

class SignalDetectTableView : public QTableView
{
    Q_OBJECT
public:
    explicit SignalDetectTableView(QWidget* = nullptr);
};

#endif // SIGNALDETECTTABLEVIEW_H
