#ifndef FILTERS_H
#define FILTERS_H

#include <algorithm>
#include <numeric>
#include <list>

struct KalmanFilter
{
    float x_mea; // measure value, instead of random number
    float e_mea; // measure offset, can not be removed
    float x_est; // estimate value
    float e_est; // estimate offset
    float Kk; // Karlman Gain

    KalmanFilter(float, float, float, float);
    void update(float);
};

struct MeanFilter
{
    float k;
    std::list<float> store;
    float v_mean;

    MeanFilter(float, float);
    void update(float);
};

struct MeanWeightedFilter
{
    float k;
    std::list<float> store;
    float v_mean;

    MeanWeightedFilter(float, float);
    void update(float);
};

#endif // FILTERS_H
