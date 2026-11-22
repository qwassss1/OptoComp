#pragma once
#include "PluginProcessor.h"

class OptoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OptoLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

private:
    void drawMainKnob (juce::Graphics& g, juce::Rectangle<float> bounds, float pos,
                       float startAngle, float endAngle, bool isVuSelector);

    void drawSmallKnob (juce::Graphics& g, juce::Rectangle<float> bounds, float pos,
                        float startAngle, float endAngle);
};

class OptoCompEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit OptoCompEditor (OptoCompProcessor&);
    ~OptoCompEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    struct RotaryControl
    {
        juce::Slider slider;
        juce::Label titleLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    enum class VuMode { Input = 0, GainReduction = 1, Output = 2 };

    void configureBigKnob (RotaryControl& ctrl, const juce::String& paramId, const juce::String& labelText);
    void configureSmallKnob (RotaryControl& ctrl, const juce::String& paramId, const juce::String& labelText);
    void drawBrushedPanel (juce::Graphics& g, juce::Rectangle<float> bounds,
                         juce::Colour topCol, juce::Colour botCol, int seed);
    void drawLogoAndTitle (juce::Graphics& g, juce::Rectangle<float> area);
    void drawScaleTicks (juce::Graphics& g, juce::Point<float> centre, float radius,
                         float startAngle, float endAngle, bool withNumbers);
    void drawVuMeter (juce::Graphics& g, juce::Rectangle<float> bounds);
    void setVuMode (VuMode newMode);

    OptoCompProcessor& proc;
    OptoLookAndFeel laf;

    RotaryControl gainKnob;
    RotaryControl peakKnob;
    RotaryControl hiFreqKnob;
    RotaryControl mixKnob;
    RotaryControl trimKnob;

    juce::ToggleButton modeSwitch { "COMPRESS / LIMIT" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> modeAttachment;

    juce::TextButton btn50Hz { "50Hz" };
    juce::TextButton btn60Hz { "60Hz" };
    juce::TextButton btnOff  { "OFF" };
    int analogMode = 2; // 0: 50Hz, 1: 60Hz, 2: OFF

    juce::Slider vuDisplayKnob;
    VuMode vuMode = VuMode::GainReduction;

    juce::Rectangle<float> topPanelBounds;
    juce::Rectangle<float> bottomPanelBounds;
    juce::Rectangle<float> vuMeterBounds;

    float displayedNeedlePos = 0.0f; // -1.0 .. +1.0
    float smoothGrDb = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptoCompEditor)
};
