#ifndef GLOBAL_H
#define GLOBAL_H

#include "TinyConfig.h"

static constexpr int BAND_WIDTH_MHZ = 20;
static constexpr int BAND_WIDTH_KHZ = 20 * 1e3;
static constexpr int BAND_WIDTH_HZ = 20 * 1e6;
static constexpr int HALF_BAND_WIDTH_MHZ = BAND_WIDTH_MHZ / 2;
static constexpr int HALF_BAND_WIDTH_KHZ = BAND_WIDTH_KHZ / 2;
static constexpr int HALF_BAND_WIDTH_HZ = BAND_WIDTH_HZ / 2;
static constexpr int CHANNEL_NUM = 4;
static constexpr int MARKER_NUM = 3;
static constexpr int ZC_NB_CHANNEL_NUM = 7;
static constexpr int ZC_NB_CHANNEL_PAGE = 4;
static constexpr int ZC_NB_CHANNEL_NUMS = ZC_NB_CHANNEL_NUM * ZC_NB_CHANNEL_PAGE;
static constexpr short AMPL_OFFSET = -160;
static constexpr short DIRECTION_ALLOW = 60 * 10;
static constexpr short BASE_DIRECTION = 131.5 * 10, UPPER_DIRECTION = BASE_DIRECTION + DIRECTION_ALLOW, LOWER_DIRECTION = BASE_DIRECTION - DIRECTION_ALLOW;
static constexpr short PHASE_MISTAKE = 2 * 10;
static constexpr int JUDGE_ARRAY_DEPTH = 320;
static constexpr int MIN_FREQ = 200;
static constexpr int MAX_FREQ = 500;
static constexpr int MIN_SAMPLE_FREQ = MIN_FREQ - BAND_WIDTH_MHZ / 2;
static constexpr int MAX_SAMPLE_FREQ = MAX_FREQ + BAND_WIDTH_MHZ / 2;
static constexpr int MIN_AMPL = -160;
static constexpr int MAX_AMPL = 0;
static constexpr int MIN_PHASE = -180;
static constexpr int MAX_PHASE = 180;
static constexpr int MARKER_MIN_DIRECTION = 0;
static constexpr int MARKER_MAX_DIRECTION = 360;
static constexpr int WATERFALL_DEPTH = 50;

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
    unsigned short Data;
    unsigned short Detect;
    float FreqRes;
    unsigned int SimBW;
    unsigned short GMode;
    signed short MGC;
    signed short AGC;
    unsigned short SmNum;
    unsigned short SmMode;
    unsigned short LmMode;
    signed short LmVal;
    unsigned short RcvMode;
    TinyConfig tinyConfig;
};

#endif // GLOBAL_H
