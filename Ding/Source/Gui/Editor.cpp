#include "Editor.hpp"
#include <iomanip>
#include <sstream>
#include "Processor.hpp"

namespace {
constexpr float aspectRatio = 6.4f;
constexpr int screenWidth = 1000;
constexpr int screenHeight = static_cast<int>(screenWidth / aspectRatio);

constexpr int c0 = 12;
constexpr int lowestNote = c0 + 2 * 12;
constexpr int highestNote = c0 + 7 * 12;
// 5 octaves, C to C
constexpr int numWhiteKeys = 36;
constexpr int keyWidth = 23;
constexpr int keyboardWidth = numWhiteKeys * keyWidth;
}  // namespace

//==============================================================================
DingEditor::DingEditor(DingProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      volume_label("VolumeLabel", "Volume"),
      keyboardComponent(p.keyboardState,
                        KeyboardComponentBase::horizontalKeyboard) {
    setSize(screenWidth, screenHeight);

    this->setupKeyboard();
    this->setupGainKnob();
    this->startTimer(400);
}

DingEditor::~DingEditor() = default;

void DingEditor::setupKeyboard() {
    addAndMakeVisible(keyboardComponent);
    keyboardComponent.setBlackNoteLengthProportion(0.6f);

    keyboardComponent.setAvailableRange(lowestNote, highestNote);
    keyboardComponent.setKeyWidth(keyWidth);
    keyboardComponent.setOctaveForMiddleC(4);
}

void DingEditor::setupGainKnob() {
    this->volume_attachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.params, DingProcessor::volume_id, this->volume_knob);

    addAndMakeVisible(volume_knob);
    volume_knob.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    constexpr int value_textbox_width = 100;
    constexpr int value_textbox_height = 25;
    volume_knob.setTextBoxStyle(
        juce::Slider::TextEntryBoxPosition::TextBoxBelow, true,
        value_textbox_width, value_textbox_height);

    volume_knob.setDoubleClickReturnValue(true, 0.5);

    addAndMakeVisible(volume_label);
    volume_label.setText("Volume", juce::dontSendNotification);
    volume_label.setColour(juce::Label::textColourId,
                           juce::Colours::lightgreen);
    volume_label.setJustificationType(juce::Justification::centred);
}

void DingEditor::resized() {
    juce::Rectangle<int> area = getLocalBounds();

    auto keyboardPanel = area.removeFromRight(keyboardWidth);
    auto sidePanel = area;

    keyboardComponent.setBounds(keyboardPanel);

    volume_label.setBounds(sidePanel.removeFromTop(24));
    volume_knob.setBounds(sidePanel);
}

juce::String VolumeKnob::getTextFromValue(const double value) {
    std::stringstream ss{};
    ss << std::fixed;
    ss << std::setprecision(1);
    ss << 10.0 * value;
    return ss.str();
}

void DingEditor::timerCallback() {
    this->keyboardComponent.grabKeyboardFocus();
    this->stopTimer();
}

//==============================================================================
void DingEditor::paint(juce::Graphics& g) {
    // (Our component is opaque, so we must completely fill the background with
    // a solid colour)
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
