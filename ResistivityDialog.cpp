#include "ResistivityDialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QGroupBox>

ResistivityDialog::ResistivityDialog(QWidget* parent): QDialog(parent)
{
    qRegisterMetaType<ResistivitySet>("ResistivitySet");

    auto verticalLayout = new QVBoxLayout(this);

    auto groupBox = new QGroupBox("地阻");
    auto formLayout = new QFormLayout(groupBox);
    for (auto i = 0; i < ResistivitySet::DIRECTIONS; ++i)
    {
        formLayout->addRow(new QLabel(QString("测量值R%1(Ω):").arg(i + 1)), ResistanceEdit[i] = new QDoubleSpinBox);
        formLayout->addRow(new QLabel(QString("土壤电阻率ρ%1(Ω•m):").arg(i + 1)), ResistivityEdit[i] = new QDoubleSpinBox);
    }
    formLayout->addRow(new QLabel("平均值ρ ̅(Ω•m):"), AverageResistivityEdit = new QDoubleSpinBox);
    verticalLayout->addWidget(groupBox);

    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Confirm = new QPushButton("确认"));
    connect(pushButton_Confirm, &QPushButton::clicked, this, [this] {
        ResistivitySet param;
        for (auto i = 0; i < ResistivitySet::DIRECTIONS; ++i)
        {
            param.Resistance[i] = ResistanceEdit[i]->value();
            param.Resistivity[i] = ResistivityEdit[i]->value();
        }
        param.AverageResistivity = AverageResistivityEdit->value();
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
