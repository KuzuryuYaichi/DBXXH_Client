#ifndef TYPICALFREQSETWIDGET_H
#define TYPICALFREQSETWIDGET_H

#include <QWidget>
#include <QMap>
#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>

class TypicalFreqSetWidget : public QDialog
{
    Q_OBJECT
public:
    explicit TypicalFreqSetWidget(QWidget *parent = nullptr);
    void SetCurrentTypicalFreqFromTable(const std::map<int, std::map<int, int>>& lst);

signals:
    void sigHaveTypicalFreq(const std::list<int>& mapValue);

private:
    void setupUi();
    static constexpr int SETTING_LINE = 7;
    QCheckBox *checkBox_Enable[SETTING_LINE];
    QDoubleSpinBox *lineEdit_TypicalFreq[SETTING_LINE];
    QPushButton *pushButton_Confirm;
    QPushButton *pushButton_Cancel;
};

#endif // TYPICALFREQSETWIDGET_H
