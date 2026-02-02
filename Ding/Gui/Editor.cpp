#include "Editor.hpp"

#include <iomanip>
#include <sstream>

#include "Processor.hpp"

namespace {
namespace impl {
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
}  // namespace impl
}  // namespace

//==============================================================================
DingEditor::DingEditor(DingProcessor& p)
    : AudioProcessorEditor(&p),
      m_audioProcessor(p),
      m_volume_label("VolumeLabel", "Volume"),
      m_keyboardComponent(p.m_keyboardState,
                          juce::KeyboardComponentBase::horizontalKeyboard)
{
    setSize(impl::screenWidth, impl::screenHeight);

    setupKeyboard();
    setupGainKnob();
    startTimer(400);
}

DingEditor::~DingEditor() = default;

void DingEditor::setupKeyboard()
{
    addAndMakeVisible(m_keyboardComponent);
    m_keyboardComponent.setBlackNoteLengthProportion(0.6f);

    m_keyboardComponent.setAvailableRange(impl::lowestNote, impl::highestNote);
    m_keyboardComponent.setKeyWidth(impl::keyWidth);
    m_keyboardComponent.setOctaveForMiddleC(4);
}

void DingEditor::setupGainKnob()
{
    m_volume_attachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            m_audioProcessor.m_params, DingProcessor::s_volume_id,
            m_volume_knob);

    addAndMakeVisible(m_volume_knob);
    m_volume_knob.setSliderStyle(
        juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    constexpr int value_textbox_width = 100;
    constexpr int value_textbox_height = 25;
    m_volume_knob.setTextBoxStyle(
        juce::Slider::TextEntryBoxPosition::TextBoxBelow, true,
        value_textbox_width, value_textbox_height);

    m_volume_knob.setDoubleClickReturnValue(true, 0.5);

    addAndMakeVisible(m_volume_label);
    m_volume_label.setText("Volume", juce::dontSendNotification);
    m_volume_label.setColour(juce::Label::textColourId,
                             juce::Colours::lightgreen);
    m_volume_label.setJustificationType(juce::Justification::centred);
}

void DingEditor::resized()
{
    juce::Rectangle<int> area = getLocalBounds();

    auto keyboardPanel = area.removeFromRight(impl::keyboardWidth);
    auto sidePanel = area;

    m_keyboardComponent.setBounds(keyboardPanel);

    m_volume_label.setBounds(sidePanel.removeFromTop(24));
    m_volume_knob.setBounds(sidePanel);
}

juce::String VolumeKnob::getTextFromValue(const double value)
{
    std::stringstream ss{};
    ss << std::fixed;
    ss << std::setprecision(1);
    ss << 10.0 * value;
    return ss.str();
}

void DingEditor::timerCallback()
{
    m_keyboardComponent.grabKeyboardFocus();
    stopTimer();
}

//==============================================================================
void DingEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with
    // a solid colour)
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
