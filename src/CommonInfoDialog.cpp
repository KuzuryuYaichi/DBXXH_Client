#include "inc/CommonInfoDialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>

CommonInfoDialog::CommonInfoDialog(QWidget *parent): QDialog(parent)
{
    qRegisterMetaType<CommonInfoSet>("CommonInfoSet");

    auto verticalLayout = new QVBoxLayout(this);

    auto groupBox = new QGroupBox("基本信息");
    auto formLayout = new QFormLayout(groupBox);
    formLayout->addRow(new QLabel("温度(℃):"), TempratureEdit = new QDoubleSpinBox);
    formLayout->addRow(new QLabel("湿度(%rh):"), HumidityEdit = new QDoubleSpinBox);
    formLayout->addRow(new QLabel("天气:"), WeatherComboBox = new QComboBox);
    WeatherComboBox->addItems( { "晴", "多云", "阴", "阵雨", "雷阵雨", "雷阵雨冰雹", "雨夹雪", "小雨", "中雨", "大雨", "暴雨", "大暴雨",
                                "特大暴雨", "阵雪", "小雪", "中雪", "大雪", "暴雪", "雾", "冻雨", "沙尘暴", "浮尘", "扬沙", "霾" } );
    WeatherComboBox->setCurrentIndex(0);
    formLayout->addRow(new QLabel("单位:"), CompanyEdit = new QLineEdit);
    formLayout->addRow(new QLabel("测试地点:"), TestPositionEdit = new QLineEdit);
    formLayout->addRow(new QLabel("测量人员:"), SurveyorEdit = new QLineEdit);
    formLayout->addRow(new QLabel("日期:"), DateEdit = new QDateEdit);
    DateEdit->setDate(QDate::currentDate());
    DateEdit->setCalendarPopup(true);
    verticalLayout->addWidget(groupBox);

    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_Confirm = new QPushButton("确认"));
    connect(pushButton_Confirm, &QPushButton::clicked, this, [this] {
        CommonInfoSet param;
        param.Temprature = TempratureEdit->value();
        param.Humidity = HumidityEdit->value();
        param.Company = CompanyEdit->text();
        param.TestPosition = TestPositionEdit->text();
        param.Surveyor = SurveyorEdit->text();
        param.Weather = WeatherComboBox->currentText();
        param.Date = DateEdit->date();

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
