#include "global.h"

PARAMETER_SET g_parameter_set = {
    .CenterFreq = MID_FREQ_HZ,
    .FreqRes = 0x0E,
    .SimBW = 2,
    .SmNum = 4,
    .Rf_MGC = 0,
    .Digit_MGC = 0,
    .GainMode = 0,
    .Feedback = 0,
    .tinyConfig = TinyConfig()
};
