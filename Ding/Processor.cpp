/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "Processor.hpp"

#include "Gui/Editor.hpp"
#include "Synth/Voice.hpp"

#include <cassert>

const std::string DingProcessor::s_volume_id = "volume";
const std::string DingProcessor::s_volume_name = "Volume";

AudioProcessorValueTreeState::ParameterLayout
DingProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto volume_parameter = std::make_unique<AudioParameterFloat>(
        s_volume_id, s_volume_name, NormalisableRange<float>(0.0f, 1.0f), 0.5f);
    params.push_back(std::move(volume_parameter));

    return {params.begin(), params.end()};
}

//==============================================================================
DingProcessor::DingProcessor()
    : AudioProcessor(
          BusesProperties().withOutput("Output",
                                       juce::AudioChannelSet::stereo(),
                                       true))

      ,
      m_params(*this,
               nullptr,
               "PARAMETERS",
               DingProcessor::createParameterLayout())
{
    static_assert(std::atomic<float>::is_always_lock_free);

    constexpr int nVoices = 16;
    for (int _ = 0; _ < nVoices; ++_) {
        m_synth.addVoice(new Voice());
    }
    m_synth.addSound(new SynthSound());
}

DingProcessor::~DingProcessor() = default;

void DingProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                 juce::MidiBuffer& midiBuffer)
{
    const auto nSamples = buffer.getNumSamples();

    buffer.clear();

    m_keyboardState.processNextMidiBuffer(midiBuffer, 0, nSamples, true);

    m_synth.renderNextBlock(buffer, midiBuffer, 0, buffer.getNumSamples());

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    const float targetVolume = m_params.getRawParameterValue(s_volume_id)
                                   ->load(std::memory_order_relaxed);

    for (auto i = 0; i < nSamples; ++i) {
        m_masterVolume =
            targetVolume + m_volumeCoeff * (m_masterVolume - targetVolume);
        leftChannel[i] *= m_masterVolume;
        rightChannel[i] *= m_masterVolume;
    }
}

void DingProcessor::prepareToPlay(const double sampleRate,
                                  const int /* samplesPerBlock */)
{
    m_synth.setCurrentPlaybackSampleRate(sampleRate);

    const float smoothingTime = 0.02f;  // 20 ms
    m_volumeCoeff =
        std::exp(-1.0f / (smoothingTime * static_cast<float>(sampleRate)));
}

void DingProcessor::releaseResources() {}

//================== boiler plate =============================================

const juce::String DingProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DingProcessor::acceptsMidi() const
{
    return true;
}

bool DingProcessor::producesMidi() const
{
    return false;
}

bool DingProcessor::isMidiEffect() const
{
    return false;
}

double DingProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DingProcessor::getNumPrograms()
{
    return 1;  // NB: some hosts don't cope very well if you tell them there are
               // 0 programs, so this should be at least 1, even if you're not
               // really implementing programs.
}

int DingProcessor::getCurrentProgram()
{
    return 0;
}

void DingProcessor::setCurrentProgram(const int index)
{
    (void)index;
}

const juce::String DingProcessor::getProgramName(const int index)
{
    (void)index;
    return "Ding";
}

void DingProcessor::changeProgramName(const int index,
                                      const juce::String& newName)
{
    (void)index;
    (void)newName;
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool DingProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

//==============================================================================

bool DingProcessor::hasEditor() const
{
    return true;  // (change this to false if you choose to not supply an
                  // editor)
}

juce::AudioProcessorEditor* DingProcessor::createEditor()
{
    return new DingEditor(*this);
}

//==============================================================================
void DingProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    (void)destData;
}

void DingProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory
    // block, whose contents will have been created by the getStateInformation()
    // call.
    (void)data;
    (void)sizeInBytes;
}

//==============================================================================
// This creates new instances of the plugin...
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DingProcessor();
}
