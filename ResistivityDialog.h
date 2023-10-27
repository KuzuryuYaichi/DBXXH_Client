#ifndef RESISTIVITYDIALOG_H
#define RESISTIVITYDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

struct ResistivitySet
{
    static constexpr auto DIRECTIONS = 4;
    double Resistance[DIRECTIONS] = { 0 };
    double Resistivity[DIRECTIONS] = { 0 };
    double AverageResistivity = 0;
};

Q_DECLARE_METATYPE(ResistivitySet);

class ResistivityDialog: public QDialog
{
    Q_OBJECT
public:
    explicit ResistivityDialog(QWidget* = nullptr);

signals:
    void triggerSetCompleted(const ResistivitySet&);

private:
    QPushButton *pushButton_Cancel;
    QPushButton *pushButton_Confirm;

    QDoubleSpinBox *ResistanceEdit[ResistivitySet::DIRECTIONS];
    QDoubleSpinBox *ResistivityEdit[ResistivitySet::DIRECTIONS];
    QDoubleSpinBox *AverageResistivityEdit;
};

#endif // RESISTIVITYDIALOG_H
