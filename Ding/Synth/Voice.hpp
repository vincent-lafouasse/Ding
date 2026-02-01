#pragma once

#include <array>

#include "JuceHeader.h"

#include "SineOscillator.hpp"

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

    void renderNextBlock(AudioBuffer<float>& outputBuffer,
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
    static constexpr std::size_t nModes = 6;
    static constexpr std::array<float, nModes> ratios = {1.0f,
                                                         1.6602819426474196f,
                                                         2.324633618320352f,
                                                         2.9888043230078396f,
                                                         3.6529839071128363f,
                                                         4.338303692992025f};
    std::array<SineOscillator, nModes> oscillators;
    juce::ADSR adsr;
    float velocity = 0.0f;
};
