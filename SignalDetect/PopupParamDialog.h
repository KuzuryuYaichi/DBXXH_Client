#ifndef POPUPPARAMDIALOG_H
#define POPUPPARAMDIALOG_H

#include <QWidget>
#include <QMetaType>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QPushButton>

struct ParamSet
{
    uint FreqPointThreshold = 0;
    uint BandwidthThreshold = 0;
    float AmplThreshold = 0;

    ParamSet() = default;
    ParamSet(uint FreqPointThreshold, uint BandwidthThreshold, float AmplThreshold):
        FreqPointThreshold(FreqPointThreshold), BandwidthThreshold(BandwidthThreshold), AmplThreshold(AmplThreshold) {}
};

Q_DECLARE_METATYPE(ParamSet);

class PopupParamDialog: public QDialog
{
    Q_OBJECT
public:
    explicit PopupParamDialog(QWidget *parent = nullptr);

signals:
    void sigUpdateParam(const ParamSet&);

private:
    QDoubleSpinBox *doubleSpinBox_FreqPointThreshold, *doubleSpinBox_BandwidthThreshold, *doubleSpinBox_GateThreshold;
    QPushButton *pushButton_Cancel;
    QPushButton *pushButton_Confirm;
};

#endif // POPUPPARAMDIALOG_H
