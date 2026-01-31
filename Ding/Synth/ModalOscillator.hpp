#pragma once

#include <array>

#include "SineOscillator.hpp"

struct DampedMode {
    float ratio;
    float decay;
    float level;
};

template <std::size_t N>
struct DampedModalOscillator {
    std::array<DampedMode, N> modes;
    std::array<SineOscillator, N> oscillators;

    void setFrequency(float freq, double sampleRate)
    {
        for (std::size_t i = 0; i < N; i++) {
            const float modalFrequency = modes[i].ratio * freq;
            oscillators[i].setFrequency(modalFrequency, sampleRate);
        }
    }

    void reset()
    {
        for (const auto& osc : oscillators) {
            osc.reset();
        }
    }

    float process()
    {
        const float dummy = 0.0f;
        return dummy;
    }
};
