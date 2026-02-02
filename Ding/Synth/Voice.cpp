#include "Voice.hpp"

#include <cmath>
#include <cstdio>
#include <numeric>

namespace GlockenspielModalData {
static constexpr std::array<float, nModes> frequencyRatios = {
    1.0f,
    2.7565361290810895f,
    5.403921459425173f,
    8.932951281230347f,
    13.34429142562536f,
    18.820878932628247f,
};
}  // namespace GlockenspielModalData

namespace {
float fromDecibels(float db)
{
    return std::powf(10.0f, db / 20.0f);
}

// determines when the voice is absolutely silent and can be returned to the
// voice pool
static constexpr float silenceThresoldDecibel = -60.0f;
static const float silenceThresold = ::fromDecibels(silenceThresoldDecibel);

// the level at which the env. has _significantly_ decayed
// makes the decay time more of a tau time constant than a time to silence
static constexpr float guiDecayThreshold = -30.0f;
// will become a GUI parameter at some point
static constexpr float guiDecayMs = 1000.0f;
}  // namespace

namespace {
float computeDecayCoefficient(float decayMs, float sampleRate)
{
    // adsr[n+1] = k * adsr[n]
    // ie adsr[n] = adsr[0] k^n = k^n
    // we're looking for k such that adsr[n] = k^n = threshold
    // with n = decaySeconds * sampleRate
    // ie k = threshold^(1/n)
    const float threshold = ::fromDecibels(::guiDecayThreshold);

    const float decaySamples = (decayMs / 1000.0f) * sampleRate;
    const float decayCoeff = std::powf(threshold, 1.0f / decaySamples);

    return decayCoeff;
}
}  // namespace

void Voice::setCurrentPlaybackSampleRate(double newRate)
{
    m_decayCoeff =
        ::computeDecayCoefficient(guiDecayMs, static_cast<float>(newRate));
}

void Voice::renderNextBlock(AudioBuffer<float>& outputBuffer,
                            const int startSample,
                            const int numSamples)
{
    if (m_level <= ::silenceThresold) {
        clearCurrentNote();
        return;
    }

    const int channels = outputBuffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i) {
        auto sumModes = [](const float sum, auto& osc) {
            const float out = sum + osc.sin();
            osc.advance();
            return out;
        };
        const float total = std::accumulate(
            m_oscillators.begin(), m_oscillators.end(), 0.0f, sumModes);
        const float sample = total / static_cast<float>(s_nModes);

        const float s = sample * m_level;

        for (int ch = 0; ch < channels; ++ch) {
            outputBuffer.addSample(ch, startSample + i, s);
        }

        m_level *= m_decayCoeff;
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

    m_level = velocity;
}

void Voice::stopNote(const float /* velocity */, const bool allowTailOff)
{
    if (!allowTailOff) {
        clearCurrentNote();
    }
    // else renderBlock will take care of clearing the note
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
