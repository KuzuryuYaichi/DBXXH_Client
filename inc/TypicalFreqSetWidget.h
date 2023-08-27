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

signals:
    void sigHaveTypicalFreq(const QMap<int, int>& mapValue);

private:
    void setupUi();
    QMap<int, int> m_mapValue;
    static constexpr int SETTING_LINE = 6;
    QCheckBox *checkBox_Enable[SETTING_LINE];
    QDoubleSpinBox *lineEdit_TypicalFreq[SETTING_LINE];
    QDoubleSpinBox *lineEdit_TestFreq[SETTING_LINE];
    QPushButton *pushButton_Confirm;
    QPushButton *pushButton_Cancel;
};

#endif // TYPICALFREQSETWIDGET_H
