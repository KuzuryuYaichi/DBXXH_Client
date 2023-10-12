#include "inc/SignalDetectTableView.h"

#include <QHeaderView>
#include <QFileInfo>
#include <QCheckBox>
#include "QXlsx/xlsxdocument.h"

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
    if (!checkBox || index.column() != 8)
        return;
    qDebug() << "SetEditorData " << index.data(Qt::DisplayRole).toBool();
    checkBox->setCheckState((Qt::CheckState)index.data(Qt::DisplayRole).toBool());
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto checkBox = qobject_cast<QCheckBox*>(editor);
    if (!checkBox || index.column() != 8)
        return;
    qDebug() << "SetModelData " << index.data(Qt::DisplayRole).toBool();
    model->setData(index, checkBox->checkState());
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

//void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    verticalHeader()->hide();
    setItemDelegateForColumn(8, new CheckBoxDelegate);
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
        xlsx.write(1, col + 1, headerView->model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString(), format);
    }
    // Write table data
    for (int row = 0; row < model()->rowCount(); ++row)
    {
        for (int col = 0; col < model()->columnCount(); ++col)
        {
            auto item = model()->index(row, col);
            if (item.isValid())
                xlsx.write(row + 2, col + 1, model()->data(item), format);
        }
    }
    // Save the Excel file
    return xlsx.saveAs(fileName);
}

void SignalDetectTableView::GenerateSignalDetectTable(QAxObject* document)
{
    auto bookmark_table = document->querySubObject("Bookmarks(QVariant)", "SignalDetect");
    if (!bookmark_table || bookmark_table->isNull())
        return;
    auto rowCount = model()->rowCount(), colCount = model()->columnCount();
    QVariantList params;
    params.append(bookmark_table->querySubObject("Range")->asVariant());
    params.append(rowCount + 1);
    params.append(colCount);

    auto datatable = document->querySubObject("Tables")->querySubObject("Add(QAxObject*, int, int, QVariant&, QVariant&)", params);
    datatable->setProperty("Style", "网格型");

    for (int col = 0; col < colCount; ++col)
    {
        auto rangeTitle = datatable->querySubObject("Cell(int, int)", 1, col + 1)->querySubObject("Range");
        rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
        rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
        rangeTitle->dynamicCall("SetText(QString)", horizontalHeader()->model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString());
    }
    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < colCount; ++col)
        {
            auto item = model()->index(row, col);
            if (item.isValid())
            {
                auto rangeTitle = datatable->querySubObject("Cell(int, int)", row + 2, col + 1)->querySubObject("Range");
                rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
                rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
                rangeTitle->dynamicCall("SetText(QString)", model()->data(item));
            }
        }
    }
}
