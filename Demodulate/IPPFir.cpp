// IPPFir.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <cmath>
#include "ipp.h"
#include "ippcore.h"
#include "ippvm.h"
#include "FSKModule.h"

// ModulationIndex 调制系数
// SampleRate 采样率   sps
// CodeRate 码元速率   bps
// Fc 中心频点  Hz
// SignalFreamLength 单帧数据长度
int _2FSKTest()
{
	double f2 = 115e3;
    double f1 = 205e3;
    double Fc = (f1 + f2) / 2;
    int codeRate = 30e3;
    int sample = 1200e3;
	int dataLen = 151500;
	double moudulationIndex = (f1 - f2) / codeRate;

    FSKModule _2FskCalc(moudulationIndex, sample, codeRate, Fc, dataLen);

    std::string file_path("D:\\WorkSpace\\fsk_jietiao-master\\IPPFir\\bin\\CH1_BW384KHZ_SR1200KHZ_SD.bin");
	std::ifstream file(file_path, std::ios::binary);
	// 读取short型数据的缓冲区
    auto data = new short[dataLen];
	memset(data, 0, sizeof(short) * dataLen);
	for (int index = 0; index < dataLen; ++index)
		file.read((char*)(data + index), sizeof(short));
    file.close();

    auto Buf = new unsigned char[1e6];
    dataLen = 5120;
    int Datalen;
    _2FskCalc.ModuFun(data, dataLen, Buf, &Datalen);
    _2FskCalc.ModuFun(data + dataLen, dataLen, Buf, &Datalen);
    _2FskCalc.ModuFun(data + dataLen + dataLen, dataLen, Buf, &Datalen);
	delete[] data;
    delete[] Buf;

	return 0;
}
