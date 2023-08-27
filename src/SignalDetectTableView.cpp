#include "inc/SignalDetectTableView.h"

#include <QHeaderView>
#include <QFileInfo>
#include "QXlsx/xlsxdocument.h"

ComboBoxDelegate::ComboBoxDelegate(QObject *parent): QItemDelegate(parent) {}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItem("是");
    comboBox->addItem("否");
    return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!comboBox || index.column() != 8)
        return;
    QString currentText = index.data(Qt::DisplayRole).toString();
    int currentIndex = comboBox->findText(currentText);
    comboBox->setCurrentIndex(currentIndex);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!comboBox || index.column() != 8)
        return;
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

//void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    if (index.column() == 8)
//    {
//        QString text = index.data(Qt::DisplayRole).toString();
//        QStyleOptionViewItem newOption(option);
//        newOption.font = QFont("Microsoft YaHei", 17);
//        drawDisplay(painter, newOption, newOption.rect, text);
//    }
//    else
//        QItemDelegate::paint(painter, option, index);
//}

SignalDetectTableView::SignalDetectTableView(QWidget *parent): QTableView(parent)
{
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSortingEnabled(true);
    verticalHeader()->hide();
    setItemDelegateForColumn(8, new ComboBoxDelegate);
}

bool SignalDetectTableView::GenerateExcelTable(QString folderName)
{
    QString fileName = folderName + "/信号检测列表" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh：mm：ss") + ".xlsx";
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);
    QXlsx::Document xlsx;
    // Write column headers
    QHeaderView *headerView = horizontalHeader();
    for (int col = 0; col < model()->columnCount(); ++col)
    {
        QString headerText = headerView->model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
        xlsx.write(1, col + 1, headerText, format);
    }
    // Write table data
    for (int row = 0; row < model()->rowCount(); ++row)
    {
        for (int col = 0; col < model()->columnCount(); ++col) {
            auto item = model()->index(row, col);
            if (item.isValid()) {
                xlsx.write(row + 2, col + 1, model()->data(item), format);
            }
        }
    }
    // Save the Excel file
    return xlsx.saveAs(fileName);
}
