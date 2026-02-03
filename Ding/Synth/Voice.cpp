#include "Voice.hpp"

#include <atomic>
#include <cmath>
#include <cstdio>

#include "core/DecibelLookup.hpp"

namespace GlockenspielModalData {
static constexpr std::array<float, nModes> frequencyRatios = {
    1.0f,
    2.7565361290810895f,
    5.403921459425173f,
    8.932951281230347f,
    13.34429142562536f,
    18.820878932628247f,
};

// gets multiplied at each sample so these parameters act pretty aggressively
// the simply supported beams at 22.4% select the first and fifth partials
//
// in a perfect world, the first and fifth partials ring out forever but they
// actually lose energy to acoustic radiation (i.e. we hear them)
//
// these should probably be physics based instead of randomly tuned
static constexpr std::array<float, nModes> relativeDecays = {
    1.0f, 0.95f, 0.9f, 0.7f, 1.0f, 0.5f,
};

// depends on the strike position
// not implemented yet so 1.0f for everyone
static constexpr std::array<float, nModes> initialAmplitude = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
};

}  // namespace GlockenspielModalData

namespace {
namespace impl {
// determines when the voice is absolutely silent and can be returned to the
// voice pool
static constexpr float silenceThresoldDecibel = -60.0f;
static const float silenceThresold =
    DecibelLookup::fromDb(silenceThresoldDecibel);

// the level at which the env. has _significantly_ decayed
// makes the decay time more of a tau time constant than a time to silence
static constexpr float guiDecayThreshold = -30.0f;

// will become a GUI parameter at some point
// so let's make it look like a gui parameter
static std::atomic<float> guiDecayMs = 3000.0f;

float computeDecayCoefficient(float decayMs,
                              double sampleRate,
                              float thresholdDecibel)
{
    // adsr[n+1] = k * adsr[n]
    // ie adsr[n] = adsr[0] k^n = k^n
    // we're looking for k such that adsr[n] = k^n = threshold
    // with n = decaySeconds * sampleRate
    // ie k = threshold^(1/n)
    const float threshold = DecibelLookup::fromDb(thresholdDecibel);

    // const float decaySamples =
    // (decayMs / 1000.0f) * static_cast<float>(sampleRate);
    const float invDecaySamples =
        1000.0f / (decayMs * static_cast<float>(sampleRate));
    const float decayCoeff = std::powf(threshold, invDecaySamples);

    return decayCoeff;
}

static constexpr float nModesInv =
    1.0f / static_cast<float>(GlockenspielModalData::nModes);

// glockenspiels plays pretty high
// hard cut LPF: do not render stuff that will alias
// soft knee LPF: HF modes are hard to excite and decay very fast
static constexpr float hfHardCut = 18.0f * 1000.0f;
static constexpr float hfSoftKnee = 10.0f * 1000.0f;

// exponential rolloff + hard cut
// std::expf is fine, this should only be called on NoteOn
float hfAttenuation(float freq)
{
    if (freq >= hfHardCut) {
        return 0.0f;
    } else if (freq >= hfSoftKnee) {
        // scale [softKnee, hardCut] to [0, 1]
        float t = (freq - hfSoftKnee) / (hfHardCut - hfSoftKnee);
        return std::expf(-3.0f * t);  // e^(-3) ≈ 0.05 at hardCut
    } else {
        return 1.0f;
    }
}
}  // namespace impl
}  // namespace

void Voice::setCurrentPlaybackSampleRate(double newRate)
{
    m_sampleRate = static_cast<float>(newRate);
    for (auto& mode : m_modes) {
        mode.osc.setSampleRate(newRate);
    }
}

void Voice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                            const int startSample,
                            const int numSamples)
{
    // check the master decay env. for voice inactivity
    // samples cannot be larger than m_level
    if (m_level <= impl::silenceThresold) {
        clearCurrentNote();
        return;
    }

    const float decayMs = impl::guiDecayMs.load(std::memory_order_relaxed);
    m_decayCoeff = impl::computeDecayCoefficient(decayMs, m_sampleRate,
                                                 impl::guiDecayThreshold);

    const int channels = outputBuffer.getNumChannels();

    for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
        float sample = 0.0f;
        for (std::size_t i = 0; i < s_nModes; i++) {
            Mode& mode = m_modes[i];
            sample += mode.osc.sin() * mode.level;
            mode.osc.advance();
            mode.level *= GlockenspielModalData::relativeDecays[i];
        }
        sample *= impl::nModesInv;  // normalize using cached 1/N
                                    // makes sure sample is in [0, 1]

        // master decay enveloppe
        const float s = sample * m_level;
        m_level *= m_decayCoeff;

        for (int ch = 0; ch < channels; ++ch) {
            outputBuffer.addSample(ch, startSample + sampleIdx, s);
        }
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
        Mode& mode = m_modes[i];
        const float freq = fundamental * ratios[i];
        mode.osc.setFrequency(freq);
        mode.osc.reset();
        // hard cut around 18kHz to avoid aliasing
        // soft knee around 10kHz to attenuate the 10k-20k octave
        mode.level = GlockenspielModalData::initialAmplitude[i] *
                     impl::hfAttenuation(freq);
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
