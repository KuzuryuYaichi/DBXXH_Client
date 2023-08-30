#ifndef SIDEWIDGET_H
#define SIDEWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QPropertyAnimation>

class SideWidget: public QTextEdit
{
public:
    SideWidget(QPushButton* = nullptr, QPushButton* = nullptr, QWidget* = nullptr);

public slots:
    void updateStatus(QString);

private:
    bool m_bSideflag = true;
    QPushButton* hideBtn;
    QPushButton *clrBtn;
};

#endif // SIDEWIDGET_H
