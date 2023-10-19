#ifndef FSK_MODULE_H
#define FSK_MODULE_H

#include "ipp.h"
#include "ippcore.h"
#include "ippvm.h"
#include "ipps.h"

class FSKModule
{
private:
    int Fc;
    double SampleRate;
    int CodeRate;
    int SignalFreamLength;
    int SymbolLength;
    int FreqStep;

    Ipp64f* MixingCoefficient = nullptr;
    Ipp64f* CosMixingBuf = nullptr;
    Ipp64f* SinMixingBuf = nullptr;
    Ipp64f* IMulWave = nullptr;
    Ipp64f* QMulWave = nullptr;
    Ipp64fc* IQWave = nullptr;

    Ipp64f* phi_xum_resamp = nullptr;
    int m_iExcateWaveLen = 0;
    Ipp64f* InsideResamp = nullptr;
	int m_iInsideResampLen = 0;
    Ipp64f* InsideAbs = nullptr;
    int currentInDataLen;
    Ipp64f *LastpSrc = nullptr, *pSrc = nullptr;
    int LastCnt = 0, SyncIdex;

public:
    FSKModule() = default;
    FSKModule(double ModulationIndex, double SampleRate, int CodeRate, int Fc, int SignalFreamLength);
    ~FSKModule();
    void ModuFun(short* InBuf, int Wavelength, unsigned char* OutBuf, int* Outlengt);

private:
    int calcFilterLen(int span, int sys);
    void calcPHI(Ipp64f* phi);
	void filterForPHI(Ipp64f* phi, Ipp64f* freFir);
	float sinc(float x);
    void RootRaisedCosineFilter(float beta, int span, float T, float ts, Ipp64f* freParm);
	int SaveLastFream(Ipp64f* WaveIn, int Wavelen, int SignalLength, int Idex);
};

#endif
