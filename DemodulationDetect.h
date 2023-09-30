#ifndef DEMODULATIONDETECT_H
#define DEMODULATIONDETECT_H

class DemodulationDetect
{
public:
    DemodulationDetect();
    static void Demod();

private:
    static constexpr int WINDOW_SIZE = 128;
};

#endif // DEMODULATIONDETECT_H
