#include "DecibelLookup.hpp"

namespace {
namespace impl {
// t expected in [0, 1]
float lerp(float t, float a, float b)
{
    return a + (b - a) * t;
}
}  // namespace impl
}  // namespace

float DecibelLookup::fromDb(float db)
{
    if (db < DecibelLookup::minDb) {
        return 0.0f;
    } else if (db >= DecibelLookup::maxDb) {
        return DecibelLookup::data[DecibelLookup::dataSize - 1];
    }

    const float floatIndex = (db + DecibelLookup::minDb) / DecibelLookup::step;

    const std::size_t i = static_cast<std::size_t>(floatIndex);
    const float frac = floatIndex - static_cast<float>(i);

    // i is safe bc of the early checks
    return impl::lerp(frac, DecibelLookup::data[i], DecibelLookup::data[i + 1]);
}
