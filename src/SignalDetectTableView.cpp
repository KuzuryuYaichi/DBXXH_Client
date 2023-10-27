#include "inc/SignalDetectTableView.h"

#include <QHeaderView>
#include <QFileInfo>
#include <QCheckBox>
#include <QMessageBox>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent): QItemDelegate(parent) {}

QWidget* CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    auto checkBox = new QCheckBox(parent);
    checkBox->setTristate(false);
    return checkBox;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto checkBox = qobject_cast<QCheckBox*>(editor);
    if (!checkBox || index.column() != 7)
        return;
    qDebug() << "SetEditorData " << index.data(Qt::DisplayRole).toBool();
    checkBox->setCheckState((Qt::CheckState)index.data(Qt::DisplayRole).toBool());
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto checkBox = qobject_cast<QCheckBox*>(editor);
    if (!checkBox || index.column() != 7)
        return;
    qDebug() << "SetModelData " << index.data(Qt::DisplayRole).toBool();
    model->setData(index, checkBox->checkState());
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

SignalDetectTableView::SignalDetectTableView(QWidget *parent): QTableView(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    verticalHeader()->hide();
    setItemDelegateForColumn(7, new CheckBoxDelegate);
}
