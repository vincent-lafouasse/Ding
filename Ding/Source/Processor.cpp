/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "Processor.hpp"
#include "Gui/Editor.hpp"
#include "Synth/AudioSource.hpp"

#include <cassert>

const std::string DingProcessor::volume_id = "volume";
const std::string DingProcessor::volume_name = "Volume";

AudioProcessorValueTreeState::ParameterLayout
DingProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto volume_parameter = std::make_unique<AudioParameterFloat>(
        DingProcessor::volume_id, DingProcessor::volume_name,
        NormalisableRange<float>(0.0f, 1.0f), 0.5f);
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
      params(*this,
             nullptr,
             "PARAMETERS",
             DingProcessor::createParameterLayout()),
      synthSource(this->keyboardState)
{
    static_assert(std::atomic<float>::is_always_lock_free);
}

DingProcessor::~DingProcessor() = default;

void DingProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                 juce::MidiBuffer& /*midiBuffer*/)
{
    this->synthSource.getNextAudioBlock(
        juce::AudioSourceChannelInfo(&buffer, 0, buffer.getNumSamples()));

    const auto numSamples = buffer.getNumSamples();
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    const float targetVolume =
        this->params.getRawParameterValue(volume_id)->load(
            std::memory_order_relaxed);

    for (auto i = 0; i < numSamples; ++i) {
        leftChannel[i] *= this->masterVolume;
        rightChannel[i] *= this->masterVolume;

        if (!juce::approximatelyEqual(targetVolume, this->masterVolume)) {
            this->masterVolume = 0.5f * (targetVolume + this->masterVolume);
        }
    }
}

void DingProcessor::prepareToPlay(const double sampleRate,
                                  const int samplesPerBlock)
{
    this->synthSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void DingProcessor::releaseResources()
{
    this->synthSource.releaseResources();
}

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
