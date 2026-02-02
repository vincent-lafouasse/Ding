#pragma once

#include <array>
#include <cstddef>

class DecibelLookup {
   public:
    static float fromDb(float db);

   private:
    static constexpr float minDb = -96.0f;
    static constexpr float maxDb = 12.0f;
    static constexpr std::size_t dataSize = 2048;  // 2 pages
    static constexpr float step =
        (DecibelLookup::maxDb - DecibelLookup::minDb) /
        static_cast<float>(DecibelLookup::dataSize - 1);

    // cached to avoid float division
    static constexpr float invStep = 1.0f / DecibelLookup::step;

    static const std::array<float, dataSize> data;
};
