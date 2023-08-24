#ifndef _STRUCT_NET_COMMON_H
#define _STRUCT_NET_COMMON_H

#include <string>

static constexpr short CTRL_TYPE = 0x08FE;

#pragma pack(1)

struct DataHead
{
    unsigned int Head = 0xF99FEFFE;
    unsigned char Version = 0x30;
    unsigned char IsEnd;
    unsigned int PackLen;
    unsigned short PackNum;
    unsigned short PackType;
    unsigned char DeviceID[14];
    unsigned char TaiShiMode = 0xff;
    unsigned short UnUsed = 0xffff;
    unsigned char Security = 4;

    DataHead() = default;
    DataHead(unsigned short PackType, unsigned int PackLen): PackLen(PackLen), PackType(PackType) {}
};

struct DataEnd
{
    unsigned short DeviceID = 1;
    double Longitude = 104.2;
    double Latitude = 32.8;
    double Height = 504;
};

struct StructNBCXResult
{
    unsigned int Task;
    long long Freq;
    float FreqResolution;
    short CXType = 6;
    short CXResolution = 1;
    short CXMode = 0;
    short CXGroupNum;
    short DataPoint;
};

struct StructNBCXData
{
    long long StartTime;
    unsigned char Range;
    short Directivity;
    unsigned char Ranges[1];
};

struct StructCXData
{
    long long StartTime;
    char Range[1];
    short Directivity[1];
};

struct StructTestCXResult
{
    unsigned int Task = 400;
    short ChNum;
    long long CenterFreq;
    float FreqResolution;
    short CXGroupNum;
    short DataPoint;
};

struct ParamPowerWB
{
    long long Time;
    float Resolution;
    int DataPoint;
    int SamplePoint;
    short Window;
    long long StartFreq;
    long long StopFreq;
    int ChannelNum;
};

struct StructTestData
{
    unsigned char Range[1];
    short PhaseDifference[1];
};

struct StructCheckCXResult
{
    unsigned int Task = 400;
    int SignalNum;
    short CXType = 6;
    short CXResolution = 1;
    short CXMode = 0;
};

struct StructCheckData
{
    long long Freq;
    int BandWidth;
    short Range;
    short SNR;
    short Directivity;
    short GetCXNum;
    short Confidence;
    int KeepTime;
    long long FindTime;
};

struct StructSweepRangeDirectionData
{
    unsigned char Range;
    short Direction;
};

struct StructSweepTimeData
{
    long long StartFreq;
    long long StopFreq;
    long long Time;
};

struct StructSweepCXResult
{
    unsigned int Task = 400;
    long long StartTime;
    float FreqResolution;
    long long StartFreq;
    long long StopFreq;
    short CXType = 6;
    short CXResolution;
    int CXResultPoint;
    int TimeNum;
};

struct StructSweepCXData
{
    long long SFreq;
    long long EFreq;
    long long DataTime;
    unsigned char Range[1];
    short Directivity[4];
};

struct StructNBWaveZCResult
{
    long long StartTime;
    int NanoSeconds;
    long long Frequency;
    int BandWidth;
    int Sps;
    char Accuracy;
    char ChannelNum;
    short DataPoint;
};

struct CommandDeviceScheck
{
    unsigned int Task;
    unsigned int Sta;
    unsigned char Type[4] = {'0', '2', '0', '1'};
    unsigned char Scheck;
    unsigned char End[2] = {'\r', '\n'};
};

struct CommandWorkCtrl
{
    unsigned int Task;
    unsigned int Sta;
    unsigned char Type[4] = {'0', '2', '0', '2'};
    unsigned char WorkCtrl;
    unsigned char End[2] = {'\r', '\n'};
};

struct CommandSetCXCommand
{
    unsigned int Task;
    unsigned int Sta;
    unsigned char Type[4] = {'0', '2', '0', '3' };
    short Data;
    short Detect;
    float FreqRes;
    int SimBW;
    short GMode;
    short MGC;
    short AGC;
    short SmNum;
    short SmMode;
    short LmMode;
    short LmVal;
    short RcvMode;
    unsigned char End[2] = {'\r', '\n'};
};

struct CommandFreqSweep
{
    unsigned int Task;
    unsigned int Sta;
    unsigned char Type[4] = {'0', '2', '0', '4'};
    unsigned char Act;
    unsigned int SFreq;
    unsigned int EFreq;
    int CTime;
    unsigned char End[2] = {'\r', '\n'};
};

struct CommandNarrowCX
{
    unsigned int Task;
    unsigned int Sta;
    unsigned char Type[4] = {'0', '2', '0', '5'};
    char Act;
    unsigned int DFreq;
    int BW;
    unsigned char DFMethod;
    unsigned int FNumber;
    unsigned char End[2] = {'\r', '\n'};
};

struct CommandFixedCX
{
    unsigned int Task;
    unsigned int Sta;
    unsigned char Type[4] = {'0', '2', '0', '6'};
    unsigned char Act;
    unsigned int CFreq;
    int CTime;
    unsigned char End[2] = {'\r', '\n'};
};

struct CommandTestCXRev
{
    unsigned int Task;
    unsigned int Sta;
    unsigned char Type[4] = {'0', '2', '0', '7'};
    unsigned char Act;
    unsigned int CFreq;
    unsigned char Mode;
    int Scope;
    unsigned char End[2] = {'\r', '\n'};
};

struct StructControlRev
{
    unsigned int Task = 400;
    short ControlFlag;
    short ErrorMsg;
};

struct StructWorkCommandRev
{
    unsigned int Task;
    float FreqRes;
    int SimBW;
    short Digit_MGC;
    short Rf_MGC;
    short SmNum;
    short GainMode;
    short Feedback;
};

struct StructDeviceScheckRev
{
    unsigned int Task = 400;
    unsigned char DeviveChNum;
    unsigned int ScheckResult;
    unsigned char AGroupNum;
    unsigned int AScheckResult;
};

struct NarrowDDC
{
    short I;
    short Q;
};

#pragma pack()

struct NetCmdData
{
    int len = 0;
    char* data = nullptr;

    NetCmdData(const std::string& str): len(sizeof(DataHead) + str.size())
    {
        data = new char[len];
        if (data == nullptr)
            return;
        new (data) DataHead(CTRL_TYPE, len);
        memcpy(data + sizeof(DataHead), str.data(), str.size());
    }

    ~NetCmdData()
    {
        if (data != nullptr)
        {
            delete[] data;
        }
    }
};

#endif // STATUSINFO_H
