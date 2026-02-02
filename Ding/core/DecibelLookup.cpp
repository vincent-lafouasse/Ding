#include "DecibelLookup.hpp"

float DecibelLookup::fromDb(float db)
{
    using DL = DecibelLookup;

    if (db < DL::minDb) {
        return 0.0f;
    } else if (db >= DL::maxDb) {
        return DL::data[DL::dataSize - 1];
    }

    const float floatIndex = (db + DL::minDb) / DL::step;

    const std::size_t i = static_cast<std::size_t>(floatIndex);
    const float frac = floatIndex - static_cast<float>(i);

    // i is safe bc of the early checks
    return DL::data[i] + frac * (DL::data[i + 1] - DL::data[i]);
}
