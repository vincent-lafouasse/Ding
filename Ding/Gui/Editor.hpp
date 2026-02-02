#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "Processor.hpp"

//==============================================================================

class VolumeKnob final : public juce::Slider {
   public:
    juce::String getTextFromValue(double value) override;
};

//==============================================================================
/**
 */
class DingEditor final : public juce::AudioProcessorEditor, public juce::Timer {
   public:
    explicit DingEditor(DingProcessor&);
    ~DingEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

   private:
    void setupGainKnob();
    void setupKeyboard();

   private:
    DingProcessor& m_audioProcessor;

    VolumeKnob m_volume_knob;
    juce::Label m_volume_label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        m_volume_attachment;

    juce::MidiKeyboardComponent m_keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DingEditor)
};
