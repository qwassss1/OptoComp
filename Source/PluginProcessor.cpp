#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    constexpr double kFastAttackSec  = 0.010;  // opto cell reacts in ~10 ms
    constexpr double kFastReleaseSec = 0.060;  // first, fast part of release
    constexpr double kSlowAttackSec  = 0.100;
    constexpr double kDetectorSec    = 0.005;  // short RMS-ish detector
    constexpr double kKneeDb         = 10.0;   // very soft knee
    constexpr double kFastWeight     = 0.6;    // share of GR that releases fast

    inline double coef (double seconds, double sr) { return 1.0 - std::exp (-1.0 / (seconds * sr)); }

    // soft-knee gain computer: returns gain reduction in dB (>= 0)
    inline double computeGR (double levelDb, double threshDb, double ratio)
    {
        const double over = levelDb - threshDb;
        const double slope = 1.0 - 1.0 / ratio;
        if (over <= -kKneeDb * 0.5) return 0.0;
        if (over >= kKneeDb * 0.5)  return over * slope;
        const double x = over + kKneeDb * 0.5;
        return slope * x * x / (2.0 * kKneeDb);
    }
}

OptoCompProcessor::OptoCompProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout OptoCompProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "peak", 1 }, "Peak Reduction",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 40.0f));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "gain", 1 }, "Gain",
        NormalisableRange<float> (-12.0f, 30.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { "mode", 1 }, "Mode",
        StringArray { "Compress", "Limit" }, 0));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "tube", 1 }, "Tube Drive",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.2f));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "mix", 1 }, "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
        AudioParameterFloatAttributes().withLabel ("%")));

    return layout;
}

juce::AudioProcessorEditor* OptoCompProcessor::createEditor()
{
    return new OptoCompEditor (*this);
}

bool OptoCompProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == l.getMainInputChannelSet();
}

void OptoCompProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    aDet     = coef (kDetectorSec, sr);
    aFastAtt = coef (kFastAttackSec, sr);
    aFastRel = coef (kFastReleaseSec, sr);
    aSlowAtt = coef (kSlowAttackSec, sr);

    msq = 0.0;
    grFast = grSlow = 0.0;

    makeupSmoothed.reset (sr, 0.05);
    makeupSmoothed.setCurrentAndTargetValue (*apvts.getRawParameterValue ("gain"));
}

void OptoCompProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    if (numCh == 0 || n == 0) return;

    const double peak   = *apvts.getRawParameterValue ("peak");
    const bool   limit  = *apvts.getRawParameterValue ("mode") > 0.5f;
    const float  tube   = *apvts.getRawParameterValue ("tube");
    const float  mix    = *apvts.getRawParameterValue ("mix") * 0.01f;
    makeupSmoothed.setTargetValue (*apvts.getRawParameterValue ("gain"));

    const double threshDb = -0.5 * peak;          // Peak Reduction lowers the threshold
    const double ratio    = limit ? 10.0 : 3.0;

    // Slow release grows with how much compression has built up (program dependent): ~1 s .. ~5 s
    const double tauSlow = 1.0 + 4.0 * juce::jlimit (0.0, 1.0, grSlow / 12.0);
    const double aSlowRel = coef (tauSlow, sr);

    // tube stage constants
    const float drive = 1.0f + 3.0f * tube;
    const float bias  = 0.1f;
    const float biasOffset = std::tanh (bias);
    const float norm = 1.0f / (drive * (1.0f - biasOffset * biasOffset));

    float* ch[2] = { buffer.getWritePointer (0),
                     numCh > 1 ? buffer.getWritePointer (1) : nullptr };

    for (int i = 0; i < n; ++i)
    {
        // linked detector (max of channels)
        float pk = std::abs (ch[0][i]);
        if (ch[1] != nullptr) pk = juce::jmax (pk, std::abs (ch[1][i]));
        msq += aDet * (static_cast<double> (pk) * pk - msq);
        const double levelDb = 10.0 * std::log10 (msq + 1.0e-12);

        const double target = computeGR (levelDb, threshDb, ratio);

        grFast += (target > grFast ? aFastAtt : aFastRel) * (target - grFast);
        grSlow += (target > grSlow ? aSlowAtt : aSlowRel) * (target - grSlow);

        const double gr = kFastWeight * grFast + (1.0 - kFastWeight) * grSlow;
        const float makeupDb = makeupSmoothed.getNextValue();
        const float g = juce::Decibels::decibelsToGain (static_cast<float> (-gr) + makeupDb);

        for (int c = 0; c < numCh; ++c)
        {
            const float dry = ch[c][i];
            float wet = dry * g;

            if (tube > 0.0f)
                wet = (std::tanh (wet * drive + bias) - biasOffset) * norm;

            ch[c][i] = dry + mix * (wet - dry);
        }
    }

    grMeterDb.store (static_cast<float> (kFastWeight * grFast + (1.0 - kFastWeight) * grSlow));
}

void OptoCompProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void OptoCompProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OptoCompProcessor();
}
