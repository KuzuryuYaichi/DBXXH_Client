#pragma once

#define EXPORT extern "C" __declspec(dllexport)

// ModulationIndex 调制系数
// SampleRate 采样率   sps
// CodeRate 码元速率   bps
// Fc 中心频点  Hz
// SignalFreamLength 单帧数据长度
// FirOrder 滤波器阶数
EXPORT int _2FSKInit(double ModulationIndex, int SampleRate, int CodeRate, int Fc, int SignalFreamLength, int FirOrder);
EXPORT int _2FSKDemodulation(short* WaveIn, int WaveLength, double* WaveOutBuf, int& waveLen, unsigned char *OutBuf, int* Outlength, int* signum, int* eye_diagram_step, double* constellation, int* constellation_len);
EXPORT int _2FSKDeletMemory();
