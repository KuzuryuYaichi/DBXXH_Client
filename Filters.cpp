#include "Filters.h"

KalmanFilter::KalmanFilter(float x_mea, float e_mea, float x_est, float e_est) : x_mea(x_mea), e_mea(e_mea), x_est(x_est), e_est(e_est)
{
    Kk = e_est / (e_est + e_mea);
}

void KalmanFilter::update(float newMeaValue)
{
    x_mea = newMeaValue;
    x_est = x_est + Kk * (x_mea - x_est);
    Kk = e_est / (e_est + e_mea);
    e_est = e_est * (1 - Kk);
}

MeanFilter::MeanFilter(float k, float initVal): k(k), v_mean(initVal)
{
    store.push_back(initVal);
}

void MeanFilter::update(float newValue)
{
    if (std::abs(newValue - v_mean) > 2)
        newValue = v_mean;
    if (store.size() >= k)
        store.pop_front();
    store.push_back(newValue);
    v_mean = std::accumulate(store.begin(), store.end(), 0.0) / store.size();
}

MeanWeightedFilter::MeanWeightedFilter(float k, float initVal): k(k), v_mean(initVal)
{
    store.push_back(initVal);
}

void MeanWeightedFilter::update(float newValue)
{
    if (store.size() >= k)
        store.pop_front();
    store.push_back(newValue);
    double sum = 0;
    int totalWidget = 0;
    int i = 1;
    for (auto iter = store.begin(); iter != store.end(); ++iter)
    {
        totalWidget += i;
        sum += (*iter) * i;
        i *= 3;
    }
    v_mean = sum / totalWidget;
}
