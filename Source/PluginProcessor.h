#pragma once
#include <JuceHeader.h>

class OrbitalCompProcessor : public juce::AudioProcessor
{
public:
    OrbitalCompProcessor();
    void prepareToPlay (double sampleRate, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Orbital Comp"; }
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
    std::atomic<float> grDb { 0.0f }, outLevel { 0.0f };
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    double sr = 44100.0;
    float env = 0.0f;           // gain reduction in dB (positive)
    juce::SmoothedValue<float> mixS, outS;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrbitalCompProcessor)
};
