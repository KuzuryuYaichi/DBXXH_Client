#ifndef CHARTWIDGETNB_H
#define CHARTWIDGETNB_H

#include "ChartWidgetCombine.h"

#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>

class ChartWidgetNB: public ChartWidgetCombine
{
    Q_OBJECT
public:
    ChartWidgetNB(QString, QWidget* = nullptr);
signals:
    void triggerListening(bool);
    void ParamsChanged(unsigned long long, unsigned int, unsigned int, unsigned int);
public slots:
    void changedListening(bool);
    void changedRecording();

private:
    void ParamsChange();
    QComboBox* demodBox;
    QPushButton* playBtn;
    QPushButton* recordBtn;
    QSpinBox* cwEdit;
    bool playing = false;
    bool recording = false;
};

#endif // CHARTWIDGETNB_H
