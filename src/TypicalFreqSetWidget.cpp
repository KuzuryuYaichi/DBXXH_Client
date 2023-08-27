#include "../inc/TypicalFreqSetWidget.h"

#include <QBoxLayout>
#include <QLabel>

TypicalFreqSetWidget::TypicalFreqSetWidget(QWidget *parent): QDialog(parent)
{
    setupUi();
}

void TypicalFreqSetWidget::setupUi()
{
    auto vBoxLayout = new QVBoxLayout(this);
    for (auto i = 0; i < SETTING_LINE; ++i)
    {
        auto horizontalLayout = new QHBoxLayout;
        horizontalLayout->addWidget(checkBox_Enable[i] = new QCheckBox("启用"));
        horizontalLayout->addWidget(new QLabel("典型频率(MHz):"));
        horizontalLayout->addWidget(lineEdit_TypicalFreq[i] = new QDoubleSpinBox);
        lineEdit_TypicalFreq[i]->setMinimum(0);
        lineEdit_TypicalFreq[i]->setMaximum(30);
        horizontalLayout->addWidget(new QLabel("测试频率(MHz):"));
        horizontalLayout->addWidget(lineEdit_TestFreq[i] = new QDoubleSpinBox);
        lineEdit_TestFreq[i]->setMinimum(0);
        lineEdit_TestFreq[i]->setMaximum(30);
        vBoxLayout->addLayout(horizontalLayout);
    }
    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Confirm = new QPushButton("确认"));
    connect(pushButton_Confirm, &QPushButton::clicked, this, [this] {
        for (auto i = 0; i < SETTING_LINE; ++i)
        {
            if(checkBox_Enable[i]->isChecked())
                m_mapValue.insert(lineEdit_TypicalFreq[i]->text().toDouble() * 1e3, lineEdit_TestFreq[i]->text().toDouble() * 1e3);
        }
        emit sigHaveTypicalFreq(m_mapValue);
    });
    horizontalLayout->addWidget(pushButton_Cancel = new QPushButton("取消"));
    connect(pushButton_Cancel, &QPushButton::clicked, this, [this] {
        close();
    });
    vBoxLayout->addLayout(horizontalLayout);
}
