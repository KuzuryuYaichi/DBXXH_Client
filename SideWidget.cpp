#include "SideWidget.h"
#include <QTime>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>

SideWidget::SideWidget(QPushButton* hideBtn, QPushButton* clrBtn, QWidget* parent): QTextEdit(parent)/*, hideBtn(hideBtn), clrBtn(clrBtn)*/
{
    setReadOnly(true);
//    connect(clrBtn, &QPushButton::clicked, this, [this] {
//        clear();
//    });

//    connect(hideBtn, &QPushButton::clicked, this, [this] (bool) {
//        m_bSideflag = !m_bSideflag;
//        auto style = QApplication::style();
//        if(m_bSideflag)
//        {
//            this->hideBtn->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
//            show();
//        }
//        else
//        {
//            this->hideBtn->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
//            hide();
//        }
//    });
}

void SideWidget::updateStatus(QString strList)
{
    append(QTime::currentTime().toString() + " : " + strList);
}
