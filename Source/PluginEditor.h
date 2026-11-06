#pragma once
#include "PluginProcessor.h"

class OrbitalLnF : public juce::LookAndFeel_V4
{
public:
    bool neon = false;
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
};

class OrbitalCompEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit OrbitalCompEditor (OrbitalCompProcessor&);
    ~OrbitalCompEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    struct Knob { juce::Slider s; juce::Label name; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att; };
    OrbitalCompProcessor& proc;
    OrbitalLnF lnf;
    Knob knobs[6];
    juce::TextButton bypass, style;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypAtt;
    float gr = 0.f, lv = 0.f, orbit = 0.f;
    juce::Rectangle<int> meter;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrbitalCompEditor)
};
