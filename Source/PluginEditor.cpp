#include "PluginEditor.h"

OptoCompEditor::OptoCompEditor (OptoCompProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);

    setupKnob (peakKnob, "peak", "PEAK REDUCTION");
    setupKnob (gainKnob, "gain", "GAIN");
    setupKnob (tubeKnob, "tube", "TUBE");
    setupKnob (mixKnob,  "mix",  "MIX");

    modeBox.addItemList ({ "Compress", "Limit" }, 1);
    addAndMakeVisible (modeBox);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.apvts, "mode", modeBox);

    setSize (540, 320);
    startTimerHz (30);
}

OptoCompEditor::~OptoCompEditor()
{
    setLookAndFeel (nullptr);
}

void OptoCompEditor::setupKnob (Knob& k, const juce::String& paramId, const juce::String& text)
{
    const float pi = juce::MathConstants<float>::pi;
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    k.slider.setRotaryParameters (pi * 1.25f, pi * 2.75f, true);
    k.slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffd8c9a3));
    k.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (k.slider);

    k.label.setText (text, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setColour (juce::Label::textColourId, juce::Colour (0xffd8c9a3));
    k.label.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    addAndMakeVisible (k.label);

    k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, paramId, k.slider);
}

void OptoCompEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff2a2622));

    g.setColour (juce::Colour (0xffd8c9a3));
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("OPTOCOMP", 20, 10, 250, 30, juce::Justification::centredLeft);

    // GR meter
    auto m = meterArea.toFloat();
    g.setColour (juce::Colour (0xff0e0e0e));
    g.fillRoundedRectangle (m, 4.0f);

    const float frac = juce::jlimit (0.0f, 1.0f, shownGr / 20.0f);
    g.setColour (juce::Colour (0xffe8b04a));
    g.fillRoundedRectangle (m.withWidth (m.getWidth() * frac).reduced (2.0f), 3.0f);

    g.setColour (juce::Colour (0xffd8c9a3));
    g.setFont (11.0f);
    g.drawText ("GR " + juce::String (shownGr, 1) + " dB", meterArea.reduced (8, 0),
                juce::Justification::centredRight);
}

void OptoCompEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (30);

    auto top = area.removeFromTop (34);
    modeBox.setBounds (top.removeFromRight (120).reduced (0, 4));
    top.removeFromRight (12);
    meterArea = top.reduced (0, 4);

    area.removeFromTop (14);

    const int w = area.getWidth() / 4;
    for (auto* k : { &peakKnob, &gainKnob, &tubeKnob, &mixKnob })
    {
        auto col = area.removeFromLeft (w);
        k->label.setBounds (col.removeFromTop (20));
        k->slider.setBounds (col);
    }
}

void OptoCompEditor::timerCallback()
{
    const float target = proc.grMeterDb.load();
    shownGr += 0.35f * (target - shownGr);
    repaint (meterArea);
}
