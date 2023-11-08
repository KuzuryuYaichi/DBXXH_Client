#ifndef COMMONINFODIALOG_H
#define COMMONINFODIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

struct CommonInfoSet
{
    double Temprature = 0, Humidity = 0;
    QString Company, TestPosition, Surveyor, Weather;
    QDate Date;
};

Q_DECLARE_METATYPE(CommonInfoSet);

class CommonInfoDialog: public QDialog
{
    Q_OBJECT
public:
    explicit CommonInfoDialog(QWidget* = nullptr);

signals:
    void triggerSetCompleted(const CommonInfoSet&);

private:
    QPushButton *pushButton_Cancel;
    QPushButton *pushButton_Confirm;

    QDoubleSpinBox *TempratureEdit, *HumidityEdit;
    QLineEdit *CompanyEdit, *TestPositionEdit, *SurveyorEdit;
    QComboBox *WeatherComboBox;
    QDateEdit *DateEdit;
};

#endif // COMMONINFODIALOG_H
