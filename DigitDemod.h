#ifndef DIGITDEMOD_H
#define DIGITDEMOD_H

#include <memory>

class DigitDemod
{
public:
    DigitDemod();
    static std::unique_ptr<unsigned char[]> Demod(const std::shared_ptr<unsigned char[]>&, double);

private:
    static constexpr int WINDOW_SIZE = 128;
};

#endif // DIGITDEMOD_H
