#include "PluginProcessor.h"
#include "PluginEditor.h"

using P = juce::AudioParameterFloat;
using R = juce::NormalisableRange<float>;

juce::AudioProcessorValueTreeState::ParameterLayout OrbitalCompProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto ms = juce::AudioParameterFloatAttributes().withLabel ("ms");
    auto db = juce::AudioParameterFloatAttributes().withLabel ("dB");
    p.push_back (std::make_unique<P> (juce::ParameterID { "th", 1 }, "Threshold", R (-60.f, 0.f, 0.1f), -24.f, db));
    p.push_back (std::make_unique<P> (juce::ParameterID { "ra", 1 }, "Ratio", R (1.f, 20.f, 0.1f, 0.5f), 4.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + ":1"; })));
    p.push_back (std::make_unique<P> (juce::ParameterID { "at", 1 }, "Attack", R (0.5f, 100.f, 0.01f, 0.3f), 10.f, ms));
    p.push_back (std::make_unique<P> (juce::ParameterID { "re", 1 }, "Release", R (20.f, 1000.f, 1.f, 0.35f), 150.f, ms));
    p.push_back (std::make_unique<P> (juce::ParameterID { "mix", 1 }, "Mix", R (0.f, 100.f, 0.1f), 100.f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));
    p.push_back (std::make_unique<P> (juce::ParameterID { "out", 1 }, "Output", R (-48.f, 24.f, 0.1f), 0.f, db));
    p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "byp", 1 }, "Bypass", false));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "sk", 1 }, "Style", juce::StringArray { "Galaxy", "Neon" }, 0));
    return { p.begin(), p.end() };
}

OrbitalCompProcessor::OrbitalCompProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout()) {}

bool OrbitalCompProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto o = l.getMainOutputChannelSet();
    if (o != juce::AudioChannelSet::mono() && o != juce::AudioChannelSet::stereo()) return false;
    return o == l.getMainInputChannelSet();
}

void OrbitalCompProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate; env = 0.0f;
    mixS.reset (sr, 0.03); outS.reset (sr, 0.03);
    mixS.setCurrentAndTargetValue (*apvts.getRawParameterValue ("mix") / 100.f);
    outS.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("out")->load(), -48.f));
}

void OrbitalCompProcessor::processBlock (juce::AudioBuffer<float>& buf, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int nCh = getTotalNumInputChannels(), n = buf.getNumSamples();
    for (int c = nCh; c < getTotalNumOutputChannels(); ++c) buf.clear (c, 0, n);
    if (apvts.getRawParameterValue ("byp")->load() > 0.5f) { grDb = 0.f; outLevel = buf.getMagnitude (0, n); return; }

    const float th = *apvts.getRawParameterValue ("th"), ra = *apvts.getRawParameterValue ("ra");
    const float at = *apvts.getRawParameterValue ("at") * 0.001f, re = *apvts.getRawParameterValue ("re") * 0.001f;
    const float knee = 6.0f;
    const float aC = std::exp (-1.0f / (float) (sr * at)), rC = std::exp (-1.0f / (float) (sr * re));
    mixS.setTargetValue (*apvts.getRawParameterValue ("mix") / 100.f);
    const float outDb = *apvts.getRawParameterValue ("out");
    outS.setTargetValue (outDb <= -47.9f ? 0.f : juce::Decibels::decibelsToGain (outDb));

    float maxGr = 0.f, peak = 0.f;
    for (int i = 0; i < n; ++i)
    {
        float lvl = 0.f;
        for (int c = 0; c < nCh; ++c) lvl = std::max (lvl, std::abs (buf.getSample (c, i)));
        const float x = juce::Decibels::gainToDecibels (lvl, -120.f);
        const float over = x - th;
        float y; // static curve, soft knee
        if (2.f * over < -knee) y = x;
        else if (2.f * std::abs (over) <= knee) { float t = over + knee * 0.5f; y = x + (1.f / ra - 1.f) * t * t / (2.f * knee); }
        else y = th + over / ra;
        const float target = x - y;
        const float coef = target > env ? aC : rC;
        env = coef * env + (1.f - coef) * target;
        maxGr = std::max (maxGr, env);
        const float g = juce::Decibels::decibelsToGain (-env);
        const float m = mixS.getNextValue(), o = outS.getNextValue();
        for (int c = 0; c < nCh; ++c)
        {
            float s = buf.getSample (c, i);
            float v = (s * (1.f - m) + s * g * m) * o;
            buf.setSample (c, i, v);
            peak = std::max (peak, std::abs (v));
        }
    }
    grDb = maxGr; outLevel = peak;
}

juce::AudioProcessorEditor* OrbitalCompProcessor::createEditor() { return new OrbitalCompEditor (*this); }

void OrbitalCompProcessor::getStateInformation (juce::MemoryBlock& d)
{
    if (auto x = apvts.copyState().createXml()) copyXmlToBinary (*x, d);
}
void OrbitalCompProcessor::setStateInformation (const void* data, int size)
{
    if (auto x = getXmlFromBinary (data, size))
        if (x->hasTagName (apvts.state.getType())) apvts.replaceState (juce::ValueTree::fromXml (*x));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OrbitalCompProcessor(); }
