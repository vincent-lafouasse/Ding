#pragma once

#include "JuceHeader.h"

class SynthAudioSource final : public juce::AudioSource {
   public:
    explicit SynthAudioSource(juce::MidiKeyboardState& keyboard_state);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

   private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
};