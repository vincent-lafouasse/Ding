#pragma once

#include <array>

#include <juce_audio_basics/juce_audio_basics.h>

#include "SineOscillator.hpp"

namespace GlockenspielModalData {
static constexpr std::size_t nModes = 6;
}

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
    // this is effectively the constructor
    void setCurrentPlaybackSampleRate(double newRate) override;

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample,
                         int numSamples) override;

    void startNote(int midiNote,
                   float velocity,
                   juce::SynthesiserSound* sound,
                   int /*pitchWheelPosition*/) override;
    void stopNote(float velocity, bool allowTailOff) override;

    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;

    bool canPlaySound(juce::SynthesiserSound* sound) override;

   private:
    static constexpr std::size_t s_nModes = GlockenspielModalData::nModes;

    struct Mode {
        SineOscillator osc;
        float level;
    };
    std::array<Mode, s_nModes> m_modes;

    // master decay
    float m_decayCoeff = 1.0f;
    float m_level = 0.0f;

    float m_sampleRate =
        44100.0f;  // safeguard value but you should _really_  call
                   // setCurrentPlaybackSampleRate before doing anything
};
