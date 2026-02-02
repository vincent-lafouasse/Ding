#include "DecibelLookup.hpp"

float DecibelLookup::fromDb(float db)
{
    using Self = DecibelLookup;

    if (db < Self::minDb) {
        return 0.0f;
    } else if (db >= Self::maxDb) {
        return Self::data[Self::dataSize - 1];
    }

    const float floatIndex = (db - Self::minDb) * Self::invStep;

    const std::size_t i = static_cast<std::size_t>(floatIndex);
    const float frac = floatIndex - static_cast<float>(i);

    // i is safe bc of the early checks
    return Self::data[i] + frac * (Self::data[i + 1] - Self::data[i]);
}
