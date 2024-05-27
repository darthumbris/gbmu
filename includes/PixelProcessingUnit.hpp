#ifndef PIXELPROCESSINGUNIT_HPP
#define PIXELPROCESSINGUNIT_HPP

#include <cstdint>

class PixelProcessingUnit
{
private:
    /* data */
public:
    PixelProcessingUnit(/* args */);
    ~PixelProcessingUnit();

    void tick(uint8_t interrupt);
};

#endif