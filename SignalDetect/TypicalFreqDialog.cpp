#include "SignalDetect/TypicalFreqDialog.h"

#include <QBoxLayout>
#include <QLabel>
#include <QMessageBox>

TypicalFreqDialog::TypicalFreqDialog(QWidget *parent): QDialog(parent)
{
    auto vBoxLayout = new QVBoxLayout(this);
    for (auto i = 0; i < SETTING_LINE; ++i)
    {
        auto horizontalLayout = new QHBoxLayout;
        horizontalLayout->addWidget(checkBox_Enable[i] = new QCheckBox("启用"));
        horizontalLayout->addWidget(new QLabel("典型频率(MHz):"));
        horizontalLayout->addWidget(lineEdit_TypicalFreq[i] = new QDoubleSpinBox);
        lineEdit_TypicalFreq[i]->setMinimum(0 + 0.2);
        lineEdit_TypicalFreq[i]->setMaximum(30 - 0.2);
        vBoxLayout->addLayout(horizontalLayout);
    }
    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Confirm = new QPushButton("确认"));
    connect(pushButton_Confirm, &QPushButton::clicked, this, [this]
    {
        std::list<int> m_lstValue;
        for (auto i = 0; i < SETTING_LINE; ++i)
        {
            if (checkBox_Enable[i]->isChecked())
                m_lstValue.emplace_back(lineEdit_TypicalFreq[i]->text().toDouble() * 1e6);
        }
        if (m_lstValue.empty())
        {
            QMessageBox::information(nullptr, "典型频点设置", "无有效频点，设置失败！");
            return;
        }
        //检查设置的典型频点彼此间是否在0.4mhz以上
        for (const auto& freqA: m_lstValue)
        {
            for (const auto& freqB: m_lstValue)
            {
                if (&freqA == &freqB)
                    continue;
                if (std::abs(freqA - freqB) < 4e5)
                {
                    QMessageBox::information(nullptr, "典型频点设置", "典型频点间距需大于0.4MHz，设置失败！");
                    return;
                }
            }
        }
        emit sigHaveTypicalFreq(m_lstValue);
        close();
    });
    horizontalLayout->addWidget(pushButton_Cancel = new QPushButton("取消"));
    connect(pushButton_Cancel, &QPushButton::clicked, this, &QDialog::close);
    vBoxLayout->addLayout(horizontalLayout);
}

void TypicalFreqDialog::SetCurrentTypicalFreqFromTable(const std::map<int, std::map<int, double>>& lst)
{
    if (lst.size() > SETTING_LINE)
        return;
    int index = 0;
    for (const auto& [freq, _]: lst)
    {
        lineEdit_TypicalFreq[index]->setValue(freq / 1e6);
        checkBox_Enable[index]->setChecked(true);
        ++index;
    }
    while (index++ < SETTING_LINE)
    {
        lineEdit_TypicalFreq[index]->setValue(0);
        checkBox_Enable[index]->setChecked(false);
    }
}
