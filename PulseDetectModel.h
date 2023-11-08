#ifndef PULSEDETECTMODEL_H
#define PULSEDETECTMODEL_H

#include <QFont>
#include <QTimer>
#include <QDateTime>
#include <mutex>
#include "StructNetData.h"
#include "SignalDetect/WBSignalDetectModel.h"

struct PulseInfo
{
    int PulseWidth;
    int PulseAmpl;
    QDateTime Time;

    void AddInfo(const Pulse& pulse)
    {
        PulseWidth = pulse.PulseWidth;
        PulseAmpl = pulse.PulseAmpl;
        Time = timeConvert(pulse.Time);
    }

    QDateTime timeConvert(unsigned long long t)
    {
        auto year = (t >> 58) & 0xFF;
        year += 100;
        unsigned long long month = 0;
        int dayOffset = (t >> 49) & 0x1FF;
        auto hour = (((t >> 48) & 0x1) ? 12 : 0) + ((t >> 44) & 0xF);
        auto minute = (t >> 38) & 0x3F;
        auto second = (t >> 32) & 0x3F;
//        int millisecond = (t & 0xFFFFFFFF) / 102.4;

        static short MONTH_DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        MONTH_DAYS[1] = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ? 29 : 28;
        for (month = 0; dayOffset > 0; ++month)
        {
            if (dayOffset - MONTH_DAYS[month] > 0)
                dayOffset -= MONTH_DAYS[month];
            else
            {
                ++month;
                break;
            }
        }
        return QDateTime(QDate(70 + year, month - 1, dayOffset), QTime(hour, minute, second));
    }
};

class PulseDetectModel: public WBSignalDetectModel
{
    Q_OBJECT
public:
    explicit PulseDetectModel(QObject* = nullptr);
    void replace(Pulse*, int);
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    bool setData(const QModelIndex&, const QVariant&, int = Qt::EditRole) override;
    QVariant data(const QModelIndex&, int = Qt::DisplayRole) const override;
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex&) const override;

public slots:
    void UpdateData() override;

protected:
    std::map<long long, PulseInfo> m_Pulse;
    std::vector<std::vector<QVariant>> m_DisplayData;

private:
    static constexpr const char* HEADER_LABEL[] = { "频率(MHz)", "脉幅(dBm)", "脉宽(us)", "时间" };
    static constexpr auto LENGTH = sizeof(HEADER_LABEL) / sizeof(HEADER_LABEL[0]);
};

#endif // PULSEDETECTMODEL_H
