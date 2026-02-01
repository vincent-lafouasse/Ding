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
    float m_cosv = 1.0f;
    float m_sinv = 0.0f;

    // rotation matrix coefficients
    // cos th; -sin th
    // sin th;  cos th
    //
    // default value is I_2
    float m_sinInc = 0.0f;
    float m_cosInc = 1.0f;

    size_t m_renormTimer = 0;
    static constexpr size_t s_renormThreshold = 256;  // renorm every _ samples

    void setFrequency(float freq, double sampleRate)
    {
        const float phase_increment =
            juce::MathConstants<float>::twoPi * freq / (float)sampleRate;
        m_cosInc = std::cos(phase_increment);
        m_sinInc = std::sin(phase_increment);
    }

    void reset()
    {
        m_sinv = 0.0f;
        m_cosv = 1.0f;
        m_renormTimer = 0;
    }

    float sin() const { return m_sinv; }

    float cos() const { return m_cosv; }

    void advance()
    {
        // matrix multiplication
        // c[n+1] = m_cosInc; -m_sinInc  x  c[n]
        // s[n+1]   m_sinInc;  m_cosInc     s[n]
        m_cosv = m_cosv * m_cosInc - m_sinv * m_sinInc;
        m_sinv = m_sinv * m_cosInc + m_cosv * m_sinInc;

        m_renormTimer++;
        if (m_renormTimer >= s_renormThreshold) {
            const float norm = std::sqrt(m_sinv * m_sinv + m_cosv * m_cosv);
            m_sinv /= norm;
            m_cosv /= norm;
            m_renormTimer = 0;
        }
    }
};
