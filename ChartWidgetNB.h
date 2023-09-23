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
    ChartWidgetNB(QString, int, QWidget* = nullptr);
signals:
    void triggerListening(int, bool);
    void ParamsChanged(unsigned long long, unsigned int, unsigned int, unsigned int);
public slots:
    void changedListening(int, bool);
    void changedRecording();

private:
    void ParamsChange();
    QComboBox* demodBox;
    QPushButton* playBtn;
    QPushButton* recordBtn;
    QSpinBox* cwEdit;
    bool playing = false;
    bool recording = false;
    int index;
};

#endif // CHARTWIDGETNB_H
