#include "Voice.hpp"

#include <numeric>

namespace GlockenspielModalData {
static constexpr std::array<float, nModes> frequencyRatios = {
    1.0f,
    1.6602819426474196f,
    2.324633618320352f,
    2.9888043230078396f,
    3.6529839071128363f,
    4.338303692992025f};
}

void Voice::setCurrentPlaybackSampleRate(double newRate)
{
    this->adsr.setSampleRate(newRate);

    juce::ADSR::Parameters p;
    p.attack = 0.005f;
    p.decay = 1.0f;
    p.sustain = 0.0f;
    p.release = 0.5f;

    adsr.setParameters(p);
}

void Voice::renderNextBlock(AudioBuffer<float>& outputBuffer,
                            const int startSample,
                            const int numSamples)
{
    if (!adsr.isActive()) {
        clearCurrentNote();
        return;
    }

    const int channels = outputBuffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i) {
        auto sumModes = [](const float sum, auto& osc) {
            return sum + osc.process();
        };
        const float total = std::accumulate(oscillators.begin(),
                                            oscillators.end(), 0.0f, sumModes);
        const float sample = total / static_cast<float>(nModes);

        const float env = adsr.getNextSample();
        const float s = sample * env * velocity;

        for (int ch = 0; ch < channels; ++ch)
            outputBuffer.addSample(ch, startSample + i, s);
    }
}

void Voice::startNote(const int midiNote,
                      const float _velocity,
                      juce::SynthesiserSound* /* sound */,
                      const int /*pitchWheelPosition*/)
{
    const auto fundamental =
        static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNote));

    const std::array<float, nModes>& ratios =
        GlockenspielModalData::frequencyRatios;

    for (std::size_t i = 0; i < nModes; i++) {
        oscillators[i].setFrequency(fundamental * ratios[i], getSampleRate());
        oscillators[i].reset();
    }

    velocity = _velocity;
    adsr.noteOn();
}

void Voice::stopNote(const float /* velocity */, const bool allowTailOff)
{
    if (allowTailOff) {
        adsr.noteOff();
    } else {
        clearCurrentNote();
    }
}

void Voice::pitchWheelMoved(const int newPitchWheelValue)
{
    (void)newPitchWheelValue;
}
void Voice::controllerMoved(const int controllerNumber,
                            int const newControllerValue)
{
    (void)controllerNumber;
    (void)newControllerValue;
}

bool Voice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}
