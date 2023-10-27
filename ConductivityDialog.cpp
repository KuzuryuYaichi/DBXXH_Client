#include "ConductivityDialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QGroupBox>

ConductivityDialog::ConductivityDialog(QWidget *parent): QDialog(parent)
{
    qRegisterMetaType<ConductivitySet>("ConductivitySet");

    auto verticalLayout = new QVBoxLayout(this);

    auto groupBox = new QGroupBox("土壤导电性");
    auto formLayout = new QFormLayout(groupBox);
    for (auto i = 0; i < ConductivitySet::MEASURE_POSITION; ++i)
    {
        formLayout->addRow(new QLabel(QString("测量值R%1(Ω):").arg(i + 1)), SoilResistanceEdit[i] = new QDoubleSpinBox);
    }
    formLayout->addRow(new QLabel("均匀性:"), ConductivityEdit = new QDoubleSpinBox);
    verticalLayout->addWidget(groupBox);

    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Confirm = new QPushButton("确认"));
    connect(pushButton_Confirm, &QPushButton::clicked, this, [this] {
        ConductivitySet param;
        for (auto i = 0; i < ConductivitySet::MEASURE_POSITION; ++i)
        {
            param.SoilResistance[i] = SoilResistanceEdit[i]->value();
        }
        param.Conductivity = ConductivityEdit->value();

        emit triggerSetCompleted(param);
        close();
    });
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Cancel = new QPushButton("取消"));
    connect(pushButton_Cancel, &QPushButton::clicked, this, [this] {
        close();
    });
    horizontalLayout->addStretch();
    verticalLayout->addLayout(horizontalLayout);
}
