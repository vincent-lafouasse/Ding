#include "Voice.hpp"

#include <numeric>

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
        const float s = sample * env * level;

        for (int ch = 0; ch < channels; ++ch)
            outputBuffer.addSample(ch, startSample + i, s);
    }
}

void Voice::startNote(const int midiNote,
                      const float velocity,
                      juce::SynthesiserSound* /* sound */,
                      const int /*pitchWheelPosition*/)
{
    const auto fundamental =
        static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNote));

    for (std::size_t i = 0; i < nModes; i++) {
        oscillators[i].setFrequency(fundamental * ratios[i], getSampleRate());
        oscillators[i].reset();
    }

    level = velocity;
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
