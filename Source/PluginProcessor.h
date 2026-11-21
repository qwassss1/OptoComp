#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

// Optical-style leveling compressor in the spirit of classic T4-cell designs.
// Original DSP, not a component-level model of any specific unit.
class OptoCompProcessor : public juce::AudioProcessor
{
public:
    OptoCompProcessor();
    ~OptoCompProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "OptoComp"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> grMeterDb { 0.0f }; // for a future GUI meter

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    double sr = 44100.0;

    // detector
    double msq = 0.0;
    double aDet = 0.0;

    // two-stage optical cell
    double grFast = 0.0, grSlow = 0.0;
    double aFastAtt = 0.0, aFastRel = 0.0, aSlowAtt = 0.0;

    juce::SmoothedValue<float> makeupSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptoCompProcessor)
};
