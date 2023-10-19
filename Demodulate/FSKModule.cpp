#include "FSKModule.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numbers>

FSKModule::FSKModule(double ModulationIndex, double SampleRate, int CodeRate, int Fc, int SignalFreamLength):
    SampleRate(SampleRate), CodeRate(CodeRate), SignalFreamLength(SignalFreamLength),
    SymbolLength(SampleRate / CodeRate), FreqStep(ModulationIndex * CodeRate / 2)
{
    auto dataSpaceWithSynclen = SignalFreamLength + SymbolLength;

    MixingCoefficient = ippsMalloc_64f(dataSpaceWithSynclen);
    CosMixingBuf = ippsMalloc_64f(dataSpaceWithSynclen);
    SinMixingBuf = ippsMalloc_64f(dataSpaceWithSynclen);
    IMulWave = ippsMalloc_64f(dataSpaceWithSynclen);
    QMulWave = ippsMalloc_64f(dataSpaceWithSynclen);
    IQWave = ippsMalloc_64fc(dataSpaceWithSynclen);

    phi_xum_resamp = ippsMalloc_64f(dataSpaceWithSynclen);
    InsideResamp = ippsMalloc_64f(dataSpaceWithSynclen);
    InsideAbs = ippsMalloc_64f(dataSpaceWithSynclen);

    LastpSrc = ippsMalloc_64f(SymbolLength + SymbolLength);
    pSrc = ippsMalloc_64f((SignalFreamLength + SymbolLength));

    for (int i = 0; i < dataSpaceWithSynclen; ++i)
    {
        MixingCoefficient[i] = 2 * std::numbers::pi * Fc * (i + 1) / SampleRate;
    }
    for (int index = 0; index < dataSpaceWithSynclen; ++index)
    {
        CosMixingBuf[index] = cos(MixingCoefficient[index]);
        SinMixingBuf[index] = sin(MixingCoefficient[index]);
    }
}

FSKModule::~FSKModule()
{
    if (MixingCoefficient != nullptr)
        ippsFree(MixingCoefficient);
    if (CosMixingBuf != nullptr)
        ippsFree(CosMixingBuf);
    if (SinMixingBuf != nullptr)
        ippsFree(SinMixingBuf);
    if (IMulWave != nullptr)
        ippsFree(IMulWave);
    if (QMulWave != nullptr)
        ippsFree(QMulWave);
    if (IQWave != nullptr)
        ippsFree(IQWave);
    if (phi_xum_resamp != nullptr)
        ippsFree(phi_xum_resamp);
    if (InsideResamp != nullptr)
        ippsFree(InsideResamp);
    if (InsideAbs != nullptr)
        ippsFree(InsideAbs);
    if (LastpSrc != nullptr)
        ippsFree(LastpSrc);
    if (pSrc != nullptr)
        ippsFree(pSrc);
}

void FSKModule::ModuFun(short* InBuf, int Wavelength, unsigned char* OutBuf, int* Outlength)
{
	SyncIdex = 0;
    if (Wavelength > SignalFreamLength)
        return;

    //直接把上一次处理剩余的全部数据都接在本次处理的头部
//    ippsCopy_64f(LastpSrc, pSrc, LastCnt);
//    ippsConvert_16s64f_Sfs(InBuf, pSrc + LastCnt, Wavelength, 0);
	
//	currentInDataLen = LastCnt + Wavelength;

//	//获取信号的实部与虚部，合成IQWave效果
//    ippsMul_64f(pSrc, CosMixingBuf, IMulWave, currentInDataLen);
//    ippsMul_64f(pSrc, SinMixingBuf, QMulWave, currentInDataLen);
//	ippsRealToCplx_64f(IMulWave, QMulWave, IQWave, currentInDataLen);

//    Ipp64f* phiXsum = ippsMalloc_64f(currentInDataLen);
//    Ipp64f* phi = ippsMalloc_64f(currentInDataLen);
//    calcPHI(phi);
//    filterForPHI(phi, phiXsum);
//    ippsFree(phi);

//    int sampleRateEven = std::floor(SampleRate / CodeRate / 2) * 2;
//	//计算降采样倍数
//	int resample2 = 1;
//	for (int i = 1; i <= 20; ++i)
//	{
//		auto tmp = (double)sampleRateEven / i;
//		if (tmp == std::floor(tmp) && std::floor(tmp) >= 5 && std::floor(tmp) <= 10)
//		{
//			resample2 = i;
//			break;
//		}
//	}
//    //直接使用原始数据，不去除头部的半个码元长度的数据，若去除会出现首个码字缺失问题，造成不同步
//	int cuttedLength = currentInDataLen;
//	int phase = 0;
//    ippsSampleDown_64f(phiXsum, cuttedLength, phi_xum_resamp, &m_iExcateWaveLen, resample2, &phase);
//	int resample_rate = sampleRateEven / resample2;

//	int fine_samp_addr = -1;
//	int samp_addr = 1;
//	int calc_symb_len = 32;
//	int calc_samp_len = resample_rate * calc_symb_len;

//    std::vector<double> samp_sum(resample_rate);
//    int samp_addr_out = 0, last_samp_addr_out = 0;
//	int i = 0;
//    int dealedIndex = 0;
//    int samp_shift = 0;
//	while (samp_addr_out <= m_iExcateWaveLen - resample_rate)
//    {
//        ++i;
//        if (i + calc_samp_len - 1 <= m_iExcateWaveLen)
//		{
//			samp_addr = (i - 1) % resample_rate;
//            auto data2Deal = phi_xum_resamp + i - 1;
//			cuttedLength = calc_samp_len;
//			int phase = 0;
//			ippsSampleDown_64f(data2Deal, cuttedLength, InsideResamp, &m_iInsideResampLen, resample_rate, &phase);
//			ippsAbs_64f(InsideResamp, InsideAbs, m_iInsideResampLen);
//			Ipp64f sumRet;
//			ippsSum_64f(InsideAbs, m_iInsideResampLen, &sumRet);
//			samp_sum[samp_addr] = sumRet;
//			if (samp_addr == resample_rate - 1)
//            {
//                auto max_addr = std::distance(std::begin(samp_sum), std::max_element(std::begin(samp_sum), std::end(samp_sum)));

//				if (fine_samp_addr == -1)
//					samp_shift = 0;
//				else if (max_addr == resample_rate && fine_samp_addr == 1)
//					samp_shift = samp_shift - 1;
//				else if (max_addr - fine_samp_addr <= -1 && (max_addr - fine_samp_addr) > -5)
//					samp_shift = samp_shift - 1;
//				else if (max_addr == 1 && fine_samp_addr == resample_rate)
//					samp_shift = samp_shift + 1;
//				else if (max_addr - fine_samp_addr >= 1 && (max_addr - fine_samp_addr) < 5)
//					samp_shift = samp_shift + 1;
//				else
//					samp_shift = 0;

//				if (fine_samp_addr == -1)
//				{
//					fine_samp_addr = max_addr;
//					last_samp_addr_out = samp_addr_out;
//                    samp_addr_out = max_addr;
//				}
//				else if (samp_shift > calc_symb_len * 2)
//				{
//					samp_shift = 0;
//					fine_samp_addr = (fine_samp_addr + 1 - 1) % resample_rate + 1;
//					last_samp_addr_out = samp_addr_out;
//					samp_addr_out = samp_addr_out + resample_rate + 1;
//				}
//				else if (samp_shift < -calc_symb_len * 2)
//				{
//					samp_shift = 0;
//					fine_samp_addr = (fine_samp_addr - 1 - 1) % resample_rate + 1;
//					last_samp_addr_out = samp_addr_out;
//					samp_addr_out = samp_addr_out + resample_rate - 1;
//				}
//				else
//				{
//					last_samp_addr_out = samp_addr_out;
//					samp_addr_out = samp_addr_out + resample_rate;
//				}
//				if (samp_addr_out > m_iExcateWaveLen)
//					break;
//                if (phi_xum_resamp[samp_addr_out] >= 0)
//                    OutBuf[dealedIndex++] = 1;
//                else
//                    OutBuf[dealedIndex++] = 0;
//			}
//		}
//		else
//		{
//            i += 6;
//			last_samp_addr_out = samp_addr_out;
//            samp_addr_out = samp_addr_out + resample_rate;
//            if (phi_xum_resamp[samp_addr_out] >= 0)
//                OutBuf[dealedIndex++] = 1;
//            else
//                OutBuf[dealedIndex++] = 0;
//		}
//	}
//    *Outlength = dealedIndex - 1;

//	//LZMK: 进行调整，对倒数第二个最佳采样点后的原始数据后，再减去半个符号周期长度的数据用作下一帧数据的头部
//	SyncIdex = last_samp_addr_out * resample2;
//	SaveLastFream(pSrc, currentInDataLen, SymbolLength, SyncIdex);
//    ippsFree(phiXsum);
}

inline int FSKModule::calcFilterLen(int span, int sys)
{
	return span * sys + 1;
}

void FSKModule::calcPHI(Ipp64f* phi)
{
    static constexpr int CURRENT_FSK_TYPE = 2;
    int sps = std::floor(SampleRate / (FreqStep) / CURRENT_FSK_TYPE);
	int filterLen = calcFilterLen(8, sps);
    Ipp64f* freParm = ippsMalloc_64f(filterLen);
    RootRaisedCosineFilter(0.5, 8, SampleRate, FreqStep * CURRENT_FSK_TYPE, freParm);
	Ipp64f sumValue;
	ippsSum_64f(freParm, filterLen, &sumValue);
	ippsDivC_64f_I(sumValue, freParm, filterLen);

	const int dstLen = currentInDataLen + filterLen - 1;
	Ipp64f* pDstI = ippsMalloc_64f(dstLen);
	Ipp64f* pDstQ = ippsMalloc_64f(dstLen);
	IppEnum funCfg = (IppEnum)ippAlgDirect;
    int bufSize = 0;
    ippsConvolveGetBufferSize(dstLen, filterLen, ipp64f, funCfg, &bufSize);
    Ipp8u* pBuffer = ippsMalloc_8u(bufSize);
    ippsConvolve_64f(IMulWave, currentInDataLen, freParm, filterLen, pDstI, funCfg, pBuffer);
    ippsConvolve_64f(QMulWave, currentInDataLen, freParm, filterLen, pDstQ, funCfg, pBuffer);
	//LZMK: 卷积后前后各去除长度filterLen/2的元素
    ippsRealToCplx_64f(pDstI + filterLen / 2, pDstQ + filterLen / 2, IQWave, currentInDataLen);
	ippsFree(pDstI);
	ippsFree(pDstQ);
	ippsFree(pBuffer);
	ippsFree(freParm);

    Ipp64fc *IQWaveD0 = ippsMalloc_64fc(currentInDataLen), *IQWaveD1 = ippsMalloc_64fc(currentInDataLen);
    ippsZero_64fc(IQWaveD1, currentInDataLen);
	ippsCopy_64fc(&IQWave[0], IQWaveD0, currentInDataLen);
	ippsCopy_64fc(&IQWave[1], IQWaveD1, currentInDataLen - 1);
	ippsConj_64fc_I(IQWaveD1, currentInDataLen);

	Ipp64fc* MultiD0andConjD1 = ippsMalloc_64fc(currentInDataLen);
	ippsMul_64fc(IQWaveD0, IQWaveD1, MultiD0andConjD1, currentInDataLen);
	ippsFree(IQWaveD1);
	ippsFree(IQWaveD0);

	Ipp64f* pDstMagn = ippsMalloc_64f(currentInDataLen);
	ippsCartToPolar_64fc(MultiD0andConjD1, pDstMagn, phi, currentInDataLen);
	ippsFree(pDstMagn);
	ippsFree(MultiD0andConjD1);
}

void FSKModule::filterForPHI(Ipp64f* phi, Ipp64f* freFir)
{
    int filterLen = calcFilterLen(8, std::floor(SampleRate / CodeRate));
	Ipp64f* freParm = ippsMalloc_64f(filterLen);

    RootRaisedCosineFilter(0.5, 8, SampleRate, CodeRate, freParm);
	//当前步骤增加归一化处理
	Ipp64f sumValue;
	ippsSum_64f(freParm, filterLen, &sumValue);
	ippsDivC_64f_I(sumValue, freParm, filterLen);

	//LZMK：ippsFIRSR_64fc处理效果不合预期，采用卷积方式执行
	const int dstLen = currentInDataLen + filterLen - 1;
	Ipp64f* pDst = ippsMalloc_64f(dstLen);
	ippsZero_64f(pDst, dstLen);
	IppEnum funCfg = (IppEnum)(ippAlgDirect);

    int bufSize = 0;
	ippsConvolveGetBufferSize(dstLen, filterLen, ipp64f, funCfg, &bufSize);
	Ipp8u* pBuffer = ippsMalloc_8u(bufSize);
	ippsConvolve_64f(phi, currentInDataLen, freParm, filterLen, pDst, funCfg, pBuffer);
	ippsCopy_64f(freFir, pDst + filterLen / 2, currentInDataLen);

	ippsFree(pDst);
	ippsFree(pBuffer);
	ippsFree(freParm);
}

inline float FSKModule::sinc(float x)
{
    return x == 0.0? 1.0: std::sin(std::numbers::pi * x) / std::numbers::pi / x;
}

inline void FSKModule::RootRaisedCosineFilter(float beta, int span, float T, float ts, Ipp64f* freParm)
{
    auto N = (unsigned long)(span * T / ts + 1);
	Ipp64f* rc = freParm;
	float max = 0.0;
    float shift = -span * T / 2.0;
    for (int i = 0; i < N; i++)
	{
        auto t = shift + ts * ((float)i);
        if (std::fabs(t) == T / 2.0 / beta)
		{
            rc[i] = std::numbers::pi * sinc(1.0 / 2.0 / beta) / span / T / 2.0;
			continue;
		}
		float tv = t / T;
        float tvb = beta * tv;
        float t1 = sinc(tv) / T;
        float t2 = std::cos(std::numbers::pi * tvb);
        float t3 = 1 - std::pow(2.0 * tvb, 2);
		rc[i] = t1 * t2 / t3;
        if (rc[i] > max)
            max = rc[i];
	}
    for (int i = 0; i < N; i++)
		rc[i] /= max;
}

inline int FSKModule::SaveLastFream(Ipp64f * WaveIn, int Wavelen, int SignalLength, int Idex)
{
	LastCnt = Wavelen - Idex - SignalLength / 2;
	ippsCopy_64f(&WaveIn[Wavelen - LastCnt], LastpSrc, LastCnt);
	return 0;
}
