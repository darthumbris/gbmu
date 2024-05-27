#include "PixelProcessingUnit.hpp"

PixelProcessingUnit::PixelProcessingUnit(/* args */)
{
}

PixelProcessingUnit::~PixelProcessingUnit()
{
}

void PixelProcessingUnit::tick(uint8_t interrupt)
{
    switch (interrupt)
    {
    case 0:
        /* horizontal blank code */
        break;
    case 1:
        /* vertical blank code */
        break;
    case 2:
        /* oam scan code */
        break;
    case 3:
        /* pixel drawing code */
        break;

    default:
        break;
    }
}