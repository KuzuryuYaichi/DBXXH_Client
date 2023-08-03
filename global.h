#ifndef GLOBAL_H
#define GLOBAL_H

#include "TinyConfig.h"

static constexpr int BAND_WIDTH_MHZ = 20;
static constexpr int BAND_WIDTH_KHZ = BAND_WIDTH_MHZ * 1e3;
static constexpr int BAND_WIDTH_HZ = BAND_WIDTH_KHZ * 1e3;
static constexpr int HALF_BAND_WIDTH_MHZ = BAND_WIDTH_MHZ / 2;
static constexpr int HALF_BAND_WIDTH_KHZ = BAND_WIDTH_KHZ / 2;
static constexpr int HALF_BAND_WIDTH_HZ = BAND_WIDTH_HZ / 2;
static constexpr int CHANNEL_NUM = 4;
static constexpr int MARKER_NUM = 3;
static constexpr int ZC_NB_PAGE_NUMS = 7;
static constexpr int ZC_NB_CHANNEL_NUMS = 8;
static constexpr short AMPL_OFFSET = -160;
static constexpr short DIRECTION_ALLOW = 60 * 10;
static constexpr short BASE_DIRECTION = 130 * 10, UPPER_DIRECTION = BASE_DIRECTION + DIRECTION_ALLOW, LOWER_DIRECTION = BASE_DIRECTION - DIRECTION_ALLOW;
static constexpr short MIN_DIRECTION = 0 * 10, MAX_DIRECTION = 359.9 * 10;
static constexpr short PHASE_MISTAKE = 1 * 10;
static constexpr int JUDGE_ARRAY_DEPTH = 320;
static constexpr int MIN_FREQ = 200, MAX_FREQ = 500;
static constexpr int MIN_SAMPLE_FREQ = MIN_FREQ - HALF_BAND_WIDTH_MHZ, MAX_SAMPLE_FREQ = MAX_FREQ + HALF_BAND_WIDTH_MHZ;
static constexpr int MIN_AMPL = -160, MAX_AMPL = 0;
static constexpr int MIN_PHASE = -180, MAX_PHASE = 180;
static constexpr int MARKER_MIN_DIRECTION = 0, MARKER_MAX_DIRECTION = 360;
static constexpr int CONFIDENCE_MINIMUM = 50, CONFIDENCE_MAXIMUM = 100;
static constexpr int WATERFALL_DEPTH = 100;
static constexpr int POINTS_ANALYZE_MINIMUM = 10;
static constexpr int DECIMALS_PRECISION = 6;

enum TABLE_UPDATE_STATE {
    TABLE_INDEX_OUT_OF_RANGE,
    TABLE_POINTS_NOT_ENOUGH,
    TABLE_UPDATE_DIR_CONF,
    TABLE_CONF_NOT_ENOUGH
};

enum selfCheck {
    allCheck,
    rcvCheck,
    gatherCheck
};

enum workCtrl {
    GetParameter,
    Stop,
    Reset
};

struct PARAMETER_SET {
    float FreqRes;
    unsigned short SmNum;
    unsigned int SimBW;
    unsigned int WinType;
    signed short MGC;
    unsigned short RcvMode;
    TinyConfig tinyConfig;
};

#endif // GLOBAL_H
