#ifndef MANMADENOISETABLEVIEW_H
#define MANMADENOISETABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QAxObject>
#include <unordered_map>

class ManMadeNoiseTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ManMadeNoiseTableView(QWidget* = nullptr);
    bool GenerateExcelTable(QString, const std::unordered_map<int, int>&);
    void GenerateManMadeNoiseTable(QAxObject*, const std::unordered_map<int, int>&);
};

#endif // MANMADENOISETABLEVIEW_H
