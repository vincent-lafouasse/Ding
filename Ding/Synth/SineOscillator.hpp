#pragma once

#include "JuceHeader.h"

#include <cmath>
#include <cstddef>

// rotating a phasor around the unit circle using a rotation matrix
//
// std::sin is slower and we don't need "random access" anyways
//
// float rounding errors lead the phasor to eventually leave the unit circle so
// a renorm from time to time is needed
struct SineOscillator {
    // vector [x y]
    float cosv = 1.0f;
    float sinv = 0.0f;

    // rotation matrix coefficients
    // cos th; -sin th
    // sin th;  cos th
    float sinInc = 0.0f;
    float cosInc = 1.0f;

    size_t renorm_timer = 0;
    static constexpr size_t renorm_threshold = 256;  // renorm every _ samples

    void setFrequency(float freq, double sampleRate)
    {
        const float phase_increment =
            juce::MathConstants<float>::twoPi * freq / (float)sampleRate;
        cosInc = std::cos(phase_increment);
        sinInc = std::sin(phase_increment);
    }

    void reset()
    {
        sinv = 0.0f;
        cosv = 1.0f;
        renorm_timer = 0;
    }

    float process()
    {
        const float out = sinv;

        // matrix multiplication
        // c[n+1] = cosInc; -sinInc  x  c[n]
        // s[n+1]   sinInc;  cosInc     s[n]
        const float c = cosv * cosInc - sinv * sinInc;
        const float s = sinv * cosInc + cosv * sinInc;

        cosv = c;
        sinv = s;

        renorm_timer++;
        if (renorm_timer >= renorm_threshold) {
            const float norm = std::sqrt(s * s + c * c);
            sinv /= norm;
            cosv /= norm;
            renorm_timer = 0;
        }
        return out;
    }
};
