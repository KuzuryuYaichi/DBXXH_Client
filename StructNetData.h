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

struct ParamPowerWB
{
    long long Time;
    int Resolution;
    int DataPoint;
    int SamplePoint;
    short Window;
    long long StartFreq;
    long long StopFreq;
    int ChannelNum;
};

struct StructNBWave
{
    long long StartTime;
//    int NanoSeconds;
    unsigned short AM_DataMax;
    unsigned short AM_DC;
    long long Frequency;
    int Bandwidth;
    int Sps;
    char Accuracy;
//    char ChannelNum;
    char DataType;
    short DataPoint;
};

struct Pulse
{
    unsigned int PulseWidth; // 第2字节	脉宽的高8位（23～16） 值转换成十进制 / 96 单位：us 第3～4字节	脉宽的低16位（15～0）
    unsigned short PulseAmpl; // 第5～6字节	脉幅	直接显示
    unsigned short DDS; // 第7～8字节 频率参数	值转换成十进制 * 96 / 65536单位：MHz
    unsigned int Time; // 第9～12字节 TOA[63:32]
    unsigned int Nanoseconds; // 第13～16字节 秒内计数器
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
            delete[] data;
    }
};

#endif // STATUSINFO_H
