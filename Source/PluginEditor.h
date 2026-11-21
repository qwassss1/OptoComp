#pragma once
#include "PluginProcessor.h"

class VintageLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.0f);
        const float r = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto c = bounds.getCentre();

        g.setColour (juce::Colour (0xff141414));
        g.fillEllipse (c.x - r, c.y - r, 2.0f * r, 2.0f * r);
        g.setColour (juce::Colour (0xffd8c9a3));
        g.drawEllipse (c.x - r, c.y - r, 2.0f * r, 2.0f * r, 2.0f);

        const float angle = startAngle + pos * (endAngle - startAngle);
        juce::Path p;
        p.addRoundedRectangle (-2.0f, -r + 5.0f, 4.0f, r * 0.45f, 1.5f);
        g.fillPath (p, juce::AffineTransform::rotation (angle).translated (c));
    }
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

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void setupKnob (Knob&, const juce::String& paramId, const juce::String& text);

    OptoCompProcessor& proc;
    VintageLookAndFeel laf;

    Knob peakKnob, gainKnob, tubeKnob, mixKnob;
    juce::ComboBox modeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;

    juce::Rectangle<int> meterArea;
    float shownGr = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptoCompEditor)
};
