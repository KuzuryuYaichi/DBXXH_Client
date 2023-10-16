#include "PulseDetectModel.h"

#include <QDateTime>

PulseDetectModel::PulseDetectModel(QObject *parent): WBSignalDetectModel(parent) {}

QVariant PulseDetectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return (section < 0 || section >= LENGTH)? "": HEADER_LABEL[section];
    }
    return QVariant();
}

int PulseDetectModel::columnCount(const QModelIndex&) const
{
    return 5;
}

Qt::ItemFlags PulseDetectModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void PulseDetectModel::replace(Pulse* pulse, int len)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    for (auto i = 0; i < len; ++i)
    {
        m_Pulse[pulse->DDS].AddInfo(pulse[i]);
    }
}

void PulseDetectModel::UpdateData()
{
    static constexpr short AMPL_LIST[] =
    {
        0x000f, 0x0011, 0x0013, 0x0016, 0x0018, 0x001b, 0x001f, 0x0022, 0x0027, 0x002b, 0x0031, 0x0036, 0x003d, 0x0045, 0x004d, 0x0056,
        0x0061, 0x006d, 0x007a, 0x0089, 0x0099, 0x00ac, 0x00c1, 0x00d9, 0x00f3, 0x0111, 0x0132, 0x0158, 0x0182, 0x01b1, 0x01e5, 0x0221,
        0x0263, 0x02ae, 0x0301, 0x035f, 0x03c8, 0x043f, 0x04c3, 0x0558, 0x05ff, 0x068a, 0x078c, 0x0878, 0x0981, 0x0aa9, 0x0bf6, 0x0d6c,
        0x0f0f, 0x10e6, 0x12f6, 0x1546, 0x17de, 0x1ac8, 0x1e0d, 0x21b7, 0x25d4, 0x2a72, 0x2fa0, 0x356f, 0x3bf5, 0x4345, 0x4b7b, 0x54b0
    };
    m_DisplayData.clear();
    std::lock_guard<std::mutex> lk(m_mutex);
    beginResetModel();
    for (const auto& [freq, pulse]: m_Pulse)
    {
        std::vector<QVariant> line(LENGTH);
        line[0] = QString::number(freq * 3.0 / 2048, 'f', 6);
        line[1] = std::lower_bound(AMPL_LIST, AMPL_LIST + 63, pulse.PulseAmpl) - AMPL_LIST - 69;
        line[2] = pulse.PulseWidth / 96.0;
        line[3] = pulse.Time.toString("hh:mm:ss");
        m_DisplayData.emplace_back(std::move(line));
    }
    endResetModel();
    m_Pulse.clear();
}

