#ifndef GLOBAL_H
#define GLOBAL_H

#include "TinyConfig.h"
#include <QColor>

static constexpr int CHANNEL_NUM = 4;
static constexpr int ZC_NB_CHANNEL_NUMS = 8;
static constexpr short AMPL_OFFSET = -180;
static constexpr int DDC_LEN = 2048;
static constexpr int MIN_FREQ = 0, MAX_FREQ = 30, MID_FREQ = (MIN_FREQ + MAX_FREQ) / 2;
static constexpr int MID_FREQ_HZ = MID_FREQ * 1e6;
static constexpr int MIN_AMPL = -160, MAX_AMPL = 0, AMPL_POINTS = MAX_AMPL - MIN_AMPL;
static constexpr double MIN_PHASE = SHRT_MIN, MAX_PHASE = SHRT_MAX;
static constexpr int WATERFALL_DEPTH = 100;
static constexpr int DECIMALS_PRECISION = 6;
static constexpr char DATETIME_FORMAT[] = "yyyy-MM-dd hh:mm:ss";
static constexpr char TIME_FORMAT[] = "hh:mm:ss";
static constexpr int MARKER_NUM = 5;
static constexpr Qt::GlobalColor MARKER_COLOR[MARKER_NUM] = { Qt::red, Qt::magenta, Qt::green, Qt::darkYellow, Qt::gray };
static constexpr double NB_HALF_BANDWIDTH_MHz[] =
    { 0.375 / 2 / 1e3, 0.75 / 2 / 1e3, 1.5 / 2 / 1e3, 3.75 / 2 / 1e3, 6 / 2 / 1e3, 15 / 2 / 1e3,
     22.5 / 2 / 1e3, 37.5 / 2 / 1e3, 75 / 2 / 1e3, 125 / 2 / 1e3, 300 / 2 / 1e3, 375 / 2 / 1e3 };
static constexpr double NB_HALF_BANDWIDTH_KHz[] =
    { 0.375, 0.75, 1.5, 3.75, 6, 15, 22.5, 37.5, 75, 125, 300, 375 };
static constexpr double NB_HALF_BANDWIDTH_Hz[] =
    { 375000 / 2, 750000 / 2, 1500 / 2, 3750 / 2, 6000 / 2, 15000 / 2,
     22500 / 2, 37500 / 2, 75000 / 2, 125000 / 2, 300000 / 2, 375000 / 2 };

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
    unsigned long long CenterFreq;
    unsigned int FreqRes;
    unsigned int SimBW;
    unsigned short SmNum;
    unsigned short Rf_MGC;
    unsigned short Digit_MGC;
    unsigned char GainMode;
    unsigned char Feedback;
    TinyConfig tinyConfig;
};

#endif // GLOBAL_H
