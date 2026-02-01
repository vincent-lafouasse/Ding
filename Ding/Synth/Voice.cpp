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
    m_adsr.setSampleRate(newRate);

    juce::ADSR::Parameters p;
    p.attack = 0.005f;
    p.decay = 1.0f;
    p.sustain = 0.0f;
    p.release = 0.5f;

    m_adsr.setParameters(p);
}

void Voice::renderNextBlock(AudioBuffer<float>& outputBuffer,
                            const int startSample,
                            const int numSamples)
{
    if (!m_adsr.isActive()) {
        clearCurrentNote();
        return;
    }

    const int channels = outputBuffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i) {
        auto sumModes = [](const float sum, auto& osc) {
            return sum + osc.process();
        };
        const float total = std::accumulate(
            m_oscillators.begin(), m_oscillators.end(), 0.0f, sumModes);
        const float sample = total / static_cast<float>(s_nModes);

        const float env = m_adsr.getNextSample();
        const float s = sample * env * m_velocity;

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

    const std::array<float, s_nModes>& ratios =
        GlockenspielModalData::frequencyRatios;

    for (std::size_t i = 0; i < s_nModes; i++) {
        m_oscillators[i].setFrequency(fundamental * ratios[i], getSampleRate());
        m_oscillators[i].reset();
    }

    m_velocity = velocity;
    m_adsr.noteOn();
}

void Voice::stopNote(const float /* velocity */, const bool allowTailOff)
{
    if (allowTailOff) {
        m_adsr.noteOff();
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
