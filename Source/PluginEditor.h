#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class MidiPianoComponent : public juce::Component
{
public:
    MidiPianoComponent(TelephoneBeepAudioProcessor& p) : processor(p) {
        setInterceptsMouseClicks(true, false);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff111111));
        g.setColour(juce::Colour(0xff222222));
        g.drawRect(getLocalBounds(), 1);

        const int whiteKeys = 14; // ~2 octaves
        const float keyW = static_cast<float>(getWidth()) / whiteKeys;
        int note = 48; // C3

        for (int i = 0; i < whiteKeys; ++i) {
            auto bounds = juce::Rectangle<float>(i * keyW, 0, keyW - 1, static_cast<float>(getHeight()));
            bool isBlack = false;
            // simple white key drawing
            g.setColour(isPressed[note] ? juce::Colour(0xffff3333) : juce::Colours::white);
            g.fillRect(bounds);
            g.setColour(juce::Colours::black);
            g.drawRect(bounds, 1.0f);
            note++;
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        int note = getNoteAt(e.position);
        if (note > 0) {
            isPressed[note] = true;
            processor.triggerFromPiano(note, 0.8f);
            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent& e) override {
        int note = getNoteAt(e.position);
        if (note > 0) {
            isPressed[note] = false;
            processor.noteOffFromPiano(note);
            repaint();
        }
    }

private:
    TelephoneBeepAudioProcessor& processor;
    std::map<int, bool> isPressed;

    int getNoteAt(juce::Point<float> pos) {
        const int whiteKeys = 14;
        const float keyW = static_cast<float>(getWidth()) / whiteKeys;
        int idx = static_cast<int>(pos.x / keyW);
        if (idx < 0 || idx >= whiteKeys) return -1;
        return 48 + idx; // C3 onwards
    }
};

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

    juce::Label modeLabel;

    MidiPianoComponent piano;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> mixA, sizeA, cutA, resoA, attA, decA, susA, relA;
    std::unique_ptr<Attachment> kTuneA, kDecA, sSnapA, slToneA, slThumpA, slDecA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeA;

    void timerCallback() override;
    void updateVisibility();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TelephoneBeepAudioProcessorEditor)
};
