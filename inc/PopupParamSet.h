#ifndef POPUPPARAMSET_H
#define POPUPPARAMSET_H

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
    uint ActiveThreshold = 0;

    ParamSet() = default;
    ParamSet(uint FreqPointThreshold, uint BandwidthThreshold, float AmplThreshold, uint ActiveThreshold):
        FreqPointThreshold(FreqPointThreshold), BandwidthThreshold(BandwidthThreshold), AmplThreshold(AmplThreshold), ActiveThreshold(ActiveThreshold) {}
};

Q_DECLARE_METATYPE(ParamSet);

class PopupParamSet: public QDialog
{
    Q_OBJECT
public:
    explicit PopupParamSet(QWidget *parent = nullptr);

signals:
    void sigUpdateParam(ParamSet param);

private:
    void setupUi();
    QDoubleSpinBox *doubleSpinBox_FreqPointThreshold, *doubleSpinBox_BandwidthThreshold, *doubleSpinBox_GateThreshold;
    QSpinBox *spinBox_ActiveThreshold;
    QPushButton *pushButton_Cancel;
    QPushButton *pushButton_Confirm;
};

#endif // POPUPPARAMSET_H
