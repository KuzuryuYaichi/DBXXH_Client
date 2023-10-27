#ifndef TYPICALFREQDIALOG_H
#define TYPICALFREQDIALOG_H

#include <QWidget>
#include <QMap>
#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>

class TypicalFreqDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TypicalFreqDialog(QWidget* = nullptr);
    void SetCurrentTypicalFreqFromTable(const std::map<int, std::map<int, double>>&);

signals:
    void sigHaveTypicalFreq(const std::list<int>&);

private:
    void setupUi();
    static constexpr int SETTING_LINE = 7;
    QCheckBox *checkBox_Enable[SETTING_LINE];
    QDoubleSpinBox *lineEdit_TypicalFreq[SETTING_LINE];
    QPushButton *pushButton_Confirm;
    QPushButton *pushButton_Cancel;
};

#endif // TYPICALFREQDIALOG_H
