#ifndef STRUCTSAMPLE_H
#define STRUCTSAMPLE_H

#include "global.h"

#pragma pack(1)

struct StructChannel
{
    short I = 0;
    short Q = 0;
};

struct StructDirection
{
    static constexpr int CHANNEL_LENGTH = 8;
    StructChannel Channel[CHANNEL_LENGTH];
};

struct StructFreqPoint
{
    static constexpr int DIRECTION_LENGTH = 128;
    StructDirection Direction[DIRECTION_LENGTH];
};

struct StructSample
{
    static constexpr int TOTAL_FREQ_LENGTH = MAX_SAMPLE_FREQ - MIN_SAMPLE_FREQ + 1;
    StructFreqPoint FreqPoint[TOTAL_FREQ_LENGTH];
};

#pragma pack()

#endif // STRUCTSAMPLE_H
