#include "DigitDemod.h"
#include "StructNetData.h"
#include "global.h"

DigitDemod::DigitDemod() {}

std::unique_ptr<unsigned char[]> DigitDemod::Demod(const std::shared_ptr<unsigned char[]>& data, double CodeRate)
{
    auto param = (StructNBWave*)(data.get() + sizeof(DataHead));
    auto amplData = (NarrowDDC*)(param + 1);

    double SpsBandwidth = NB_HALF_BANDWIDTH_KHz[11];
    switch (param->Bandwidth)
    {
    case 150: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[0]; break;
    case 300: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[1]; break;
    case 600: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[2]; break;
    case 1500: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[3]; break;
    case 2400: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[4]; break;
    case 6000: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[5]; break;
    case 9000: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[6]; break;
    case 15000: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[7]; break;
    case 30000: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[8]; break;
    case 50000: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[9]; break;
    case 120000: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[10]; break;
    case 150000: SpsBandwidth = NB_HALF_BANDWIDTH_KHz[11]; break;
    }
    int PointSkip = SpsBandwidth / CodeRate;
    if (PointSkip <= 0)
        return nullptr;
    std::vector<unsigned long long> pos(PointSkip, 0);
    for (auto i = 0; i < DDC_LEN; i += PointSkip)
    {
        for (auto j = 0; j < PointSkip; ++j)
        {
            pos[j] += std::abs(amplData[i + j].I);
        }
    }
    auto index = std::max_element(pos.begin(), pos.end()) - pos.begin();
    if (index < 0 || index >= DDC_LEN)
        return nullptr;
    auto FSK_Data = std::make_unique<unsigned char[]>(sizeof(DataHead) + sizeof(StructNBWave) + sizeof(NarrowDDC) * DDC_LEN);
    std::memcpy(FSK_Data.get(), data.get(), sizeof(DataHead) + sizeof(StructNBWave));
    auto& FSK_index = ((StructNBWave*)(FSK_Data.get() + sizeof(DataHead)))->DataPoint = 0;
    auto pDst = (NarrowDDC*)(FSK_Data.get() + sizeof(DataHead) + sizeof(StructNBWave));
    for (auto i = index; i < DDC_LEN; i += PointSkip)
    {
        pDst[FSK_index++] = amplData[i];
    }
    return FSK_Data;
}
