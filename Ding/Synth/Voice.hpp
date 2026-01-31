#pragma once

#include "JuceHeader.h"

// rotating a phasor around the unit circle using a rotation matrix
//
// std::sin is slower and we don't need "random access" anyways
//
// float rounding errors lead result to eventually leave the unit circle so a
// renorm from time to time is needed
struct SineOscillator {
    float sinv = 0.0f;
    float cosv = 1.0f;

    float sinInc = 0.0f;
    float cosInc = 1.0f;

    size_t renorm_timer = 0;
    static constexpr size_t renorm_timer_mask =
        0xff;  // renorm every 256 samples

    void setFrequency(float freq, double sampleRate)
    {
        const float w =
            juce::MathConstants<float>::twoPi * freq / (float)sampleRate;
        sinInc = std::sin(w);
        cosInc = std::cos(w);
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

        const float s = sinv * cosInc + cosv * sinInc;
        const float c = cosv * cosInc - sinv * sinInc;

        sinv = s;
        cosv = c;

        if ((renorm_timer & renorm_timer_mask) == 0) {
            const float norm = std::sqrt(s * s + c * c);
            sinv /= norm;
            cosv /= norm;
            renorm_timer = 0;
        }
        renorm_timer++;

        return out;
    }
};

class SynthSound final : public juce::SynthesiserSound {
   public:
    SynthSound() = default;
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
    ~SynthSound() override = default;
};

class Voice final : public juce::SynthesiserVoice {
   public:
    Voice() = default;
    void renderNextBlock(AudioBuffer<float>& outputBuffer,
                         int startSample,
                         int numSamples) override;

    void startNote(int midiNote,
                   float velocity,
                   juce::SynthesiserSound* sound,
                   int /*pitchWheelPosition*/) override;
    void stopNote(float velocity, bool allowTailOff) override;

    void setCurrentPlaybackSampleRate(double newRate) override;

    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;

    bool canPlaySound(juce::SynthesiserSound* sound) override;

   private:
    SineOscillator osc;
    juce::ADSR adsr;
    float level = 0.0f;
};
