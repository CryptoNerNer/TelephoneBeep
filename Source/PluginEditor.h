#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TelephoneBeepAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    TelephoneBeepAudioProcessorEditor(TelephoneBeepAudioProcessor&);
    ~TelephoneBeepAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TelephoneBeepAudioProcessor& audioProcessor;

    std::array<juce::TextButton, 12> keyButtons;
    juce::Slider reverbMixSlider, reverbSizeSlider, toneLengthSlider;
    juce::Label reverbMixLabel, reverbSizeLabel, toneLengthLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> mixAttach, sizeAttach, lengthAttach;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TelephoneBeepAudioProcessorEditor)
};
