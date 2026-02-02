#pragma once

#include <array>
#include <cstddef>

class DecibelLookup {
   private:
    static constexpr float minDb = -96.0f;
    static constexpr float maxDb = 12.0f;
    static constexpr std::size_t dataSize = 2048;  // 2 pages

    static const std::array<float, dataSize> data;
};
