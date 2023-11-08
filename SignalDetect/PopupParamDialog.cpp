#include "SignalDetect/PopupParamDialog.h"

#include <QFormLayout>
#include <QLabel>

int g_FreqPointThreshold = 10000; //Hz
int g_BandwidthThreshold = 10000; //Hz
float g_AmplThreshold = -60; //dBm

PopupParamDialog::PopupParamDialog(QWidget *parent): QDialog(parent)
{
    qRegisterMetaType<ParamSet>("ParamSet");

    auto formLayout = new QFormLayout;
    formLayout->addRow(new QLabel("载波检测频点匹配差值(kHz):"), doubleSpinBox_FreqPointThreshold = new QDoubleSpinBox);
    doubleSpinBox_FreqPointThreshold->setMinimum(0);
    doubleSpinBox_FreqPointThreshold->setMaximum(999);
    formLayout->addRow(new QLabel("载波检测带宽匹配差值(kHz):"), doubleSpinBox_BandwidthThreshold = new QDoubleSpinBox);
    doubleSpinBox_FreqPointThreshold->setMinimum(0);
    doubleSpinBox_BandwidthThreshold->setMaximum(999);
    formLayout->addRow(new QLabel("载波检测门限(dBm):"), doubleSpinBox_GateThreshold = new QDoubleSpinBox);
    doubleSpinBox_GateThreshold->setMinimum(-90);
    doubleSpinBox_GateThreshold->setMaximum(0);

    doubleSpinBox_BandwidthThreshold->setValue(g_FreqPointThreshold / 1e3);
    doubleSpinBox_FreqPointThreshold->setValue(g_BandwidthThreshold / 1e3);
    doubleSpinBox_GateThreshold->setValue(g_AmplThreshold);

    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Confirm = new QPushButton("确认"));
    connect(pushButton_Confirm, &QPushButton::clicked, this, [this] {
        ParamSet param(doubleSpinBox_FreqPointThreshold->value() * 1e3, doubleSpinBox_BandwidthThreshold->value() * 1e3, doubleSpinBox_GateThreshold->value());
        emit sigUpdateParam(param);
        close();
      });
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Cancel = new QPushButton("取消"));
    connect(pushButton_Cancel, &QPushButton::clicked, this, [this] {
       close();
    });
    horizontalLayout->addStretch();

    auto verticalLayout = new QVBoxLayout(this);
    verticalLayout->addLayout(formLayout);
    verticalLayout->addLayout(horizontalLayout);
}
