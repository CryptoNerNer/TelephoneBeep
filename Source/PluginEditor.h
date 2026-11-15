#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
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

    juce::ComboBox modeBox;
    std::array<juce::TextButton, 12> keyButtons;
    juce::TextButton kickBtn, snareBtn, hatBtn, slapBtn;

    juce::Slider reverbMixSlider, reverbSizeSlider;
    juce::Slider moogCutoffSlider, moogResoSlider, moogAttackSlider, moogDecaySlider, moogSustainSlider, moogReleaseSlider;
    juce::Slider kickTuneSlider, kickDecaySlider, snareSnapSlider;
    juce::Slider slapToneSlider, slapThumpSlider, slapDecaySlider;

    juce::Label modeLabel, reverbLabel, moogLabel, drumLabel, slapLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> mixA, sizeA, cutA, resoA, attA, decA, susA, relA;
    std::unique_ptr<Attachment> kTuneA, kDecA, sSnapA, slToneA, slThumpA, slDecA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeA;

    void timerCallback() override;
    void updateVisibility();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TelephoneBeepAudioProcessorEditor)
};
