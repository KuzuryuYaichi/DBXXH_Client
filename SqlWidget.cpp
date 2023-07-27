#include "SqlWidget.h"

#include "global.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

SqlWidget::SqlWidget(QWidget* parent): QWidget(parent), sqlData("Query", tableView = new QTableView)
{
    auto hBoxLayout = new QHBoxLayout;

    auto QueryConditionBox = new QGroupBox(tr("Query Condition"));
    auto QueryConditionLayout = new QHBoxLayout(QueryConditionBox);
    QueryConditionLayout->addWidget(new QLabel(tr("Start Freq(MHz)")));
    QueryConditionLayout->addWidget(startFreq = new QDoubleSpinBox);
    startFreq->setMinimum(MIN_FREQ);
    startFreq->setMaximum(MAX_FREQ);
    startFreq->setValue(MIN_FREQ);
    startFreq->setSingleStep(1);
    startFreq->setDecimals(3);
    startFreq->setAlignment(Qt::AlignRight);
    QueryConditionLayout->addStretch(100);

    QueryConditionLayout->addWidget(new QLabel(tr("End Freq(MHz)")));
    QueryConditionLayout->addWidget(endFreq = new QDoubleSpinBox);
    endFreq->setMinimum(MIN_FREQ);
    endFreq->setMaximum(MAX_FREQ);
    endFreq->setValue(MAX_FREQ);
    endFreq->setSingleStep(1);
    endFreq->setDecimals(3);
    endFreq->setAlignment(Qt::AlignRight);
    QueryConditionLayout->addStretch(100);

    QueryConditionLayout->addWidget(new QLabel(tr("Confidence Gate")));
    QueryConditionLayout->addWidget(confidenceGate = new QSpinBox);
    confidenceGate->setValue(50);
    confidenceGate->setMinimum(0);
    confidenceGate->setMaximum(100);
    confidenceGate->setSingleStep(1);
    confidenceGate->setAlignment(Qt::AlignRight);
    QueryConditionLayout->addStretch(100);

    auto now = QDateTime::currentDateTime();
    QueryConditionLayout->addWidget(new QLabel(tr("Start Time")));
    QueryConditionLayout->addWidget(startTime = new QDateTimeEdit(now));
    QueryConditionLayout->addStretch(100);

    QueryConditionLayout->addWidget(new QLabel(tr("End Time")));
    QueryConditionLayout->addWidget(endTime = new QDateTimeEdit(now));
    QueryConditionLayout->addStretch(100);

    auto QueryOperationBox = new QGroupBox(tr("Query Operation"));
    auto QueryOperationLayout = new QHBoxLayout(QueryOperationBox);
    auto queryBtn = new QPushButton(tr("Query"));
    connect(queryBtn, &QPushButton::clicked, this, [this] {
        auto count = sqlData.QueryCount(startFreq->value(), endFreq->value(), confidenceGate->value(), startTime->dateTime(), endTime->dateTime());
        if (count < 0)
            return;
        auto PageCount = count / PAGE_COUNT + (count % PAGE_COUNT? 1: 0);
        pageTotal->setText(" / " + QString::number(PageCount));
        pageNow->setMaximum(PageCount);
        if (pageNow->value() > PageCount)
            pageNow->setValue(PageCount);
        sqlData.Query(startFreq->value(), endFreq->value(), confidenceGate->value(), startTime->dateTime(), endTime->dateTime(), (pageNow->value() - 1) * PAGE_COUNT);
    });
    QueryOperationLayout->addWidget(queryBtn);
    QueryOperationLayout->addStretch(100);

    QueryOperationLayout->addWidget(new QLabel(tr("Page")));
    pageNow = new QSpinBox;
    pageNow->setValue(0);
    pageNow->setMinimum(1);
    pageNow->setMaximum(1);
    pageNow->setSingleStep(1);
    QueryOperationLayout->addWidget(pageNow);
    QueryOperationLayout->addWidget(pageTotal = new QLabel(" / 1"));

    hBoxLayout->addWidget(QueryConditionBox, 7);
    hBoxLayout->addWidget(QueryOperationBox, 1);

    auto vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->addLayout(hBoxLayout);
    vBoxLayout->addWidget(tableView);
}

SqlWidget::~SqlWidget()
{

}
