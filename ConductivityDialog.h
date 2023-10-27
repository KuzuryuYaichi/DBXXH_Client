#ifndef CONDUCTIVITYDIALOG_H
#define CONDUCTIVITYDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

struct ConductivitySet
{
    static constexpr auto MEASURE_POSITION = 5;
    double SoilResistance[MEASURE_POSITION] = { 0 };
    double Conductivity = 0;
};

Q_DECLARE_METATYPE(ConductivitySet);

class ConductivityDialog: public QDialog
{
    Q_OBJECT
public:
    explicit ConductivityDialog(QWidget* = nullptr);

signals:
    void triggerSetCompleted(const ConductivitySet&);

private:
    QPushButton *pushButton_Cancel;
    QPushButton *pushButton_Confirm;

    QDoubleSpinBox *SoilResistanceEdit[ConductivitySet::MEASURE_POSITION];
    QDoubleSpinBox *ConductivityEdit;
};

#endif // CONDUCTIVITYDIALOG_H
