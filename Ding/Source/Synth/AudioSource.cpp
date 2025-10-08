#include "AudioSource.hpp"

#include "Voice.hpp"

SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState& keyboard_state)
    : keyboardState(keyboard_state) {
    constexpr int nVoices = 6;
    for (int _ = 0; _ < nVoices; ++_) {
        synth.addVoice(new Voice());
    }
    synth.addSound(new SynthSound());
}

void SynthAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/,
                                     double sampleRate) {
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SynthAudioSource::releaseResources() {}

void SynthAudioSource::getNextAudioBlock(
    const AudioSourceChannelInfo& bufferToFill) {
    bufferToFill.clearActiveBufferRegion();
    juce::MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample,
                                        bufferToFill.numSamples, true);
    synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
                          bufferToFill.startSample, bufferToFill.numSamples);
}
