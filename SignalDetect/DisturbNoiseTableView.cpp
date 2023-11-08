#include "SignalDetect/DisturbNoiseTableView.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>

LineEditDelegate::LineEditDelegate(QObject* parent): QItemDelegate(parent) {}

QWidget* LineEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    return new QLineEdit(parent);
}

void LineEditDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit || index.column() != 4)
        return;
    lineEdit->setText(index.data(Qt::DisplayRole).toString());
}

void LineEditDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit || index.column() != 4)
        return;
    model->setData(index, lineEdit->text());
}

void LineEditDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}

DisturbNoiseTableView::DisturbNoiseTableView(QWidget *parent): QTableView (parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    verticalHeader()->hide();
    setItemDelegateForColumn(4, new LineEditDelegate);
}
