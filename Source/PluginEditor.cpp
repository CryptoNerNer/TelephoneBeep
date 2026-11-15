#include "PluginEditor.h"

static const char* keyLabels[12] = { "1","2","3","4","5","6","7","8","9","*","0","#" };

TelephoneBeepAudioProcessorEditor::TelephoneBeepAudioProcessorEditor(TelephoneBeepAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(360, 520);
    setResizable(false, false);

    for (int i = 0; i < 12; ++i) {
        keyButtons[i].setButtonText(keyLabels[i]);
        keyButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222));
        keyButtons[i].setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00cc66));
        keyButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        keyButtons[i].setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        keyButtons[i].onClick = [this, i]() { audioProcessor.triggerTone(i); };
        addAndMakeVisible(keyButtons[i]);
    }

    auto setupSlider = [&](juce::Slider& s, juce::Label& l, const juce::String& name) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 22);
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00aa55));
        addAndMakeVisible(s);
        l.setText(name, juce::dontSendNotification);
        l.setColour(juce::Label::textColourId, juce::Colours::white);
        l.setFont(juce::FontOptions(15.0f));
        addAndMakeVisible(l);
    };

    setupSlider(reverbMixSlider, reverbMixLabel, "Reverb Mix");
    setupSlider(reverbSizeSlider, reverbSizeLabel, "Reverb Size");
    setupSlider(toneLengthSlider, toneLengthLabel, "Tone Length");

    mixAttach    = std::make_unique<Attachment>(audioProcessor.parameters, "reverbMix", reverbMixSlider);
    sizeAttach   = std::make_unique<Attachment>(audioProcessor.parameters, "reverbSize", reverbSizeSlider);
    lengthAttach = std::make_unique<Attachment>(audioProcessor.parameters, "toneLength", toneLengthSlider);

    startTimerHz(30);
}

TelephoneBeepAudioProcessorEditor::~TelephoneBeepAudioProcessorEditor() {}

void TelephoneBeepAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff141414));
    g.setColour(juce::Colour(0xff00cc66));
    g.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    g.drawText("TELEPHONE BEEP", 0, 12, getWidth(), 32, juce::Justification::centred);
    g.setFont(juce::FontOptions(14.0f));
    g.setColour(juce::Colours::lightgrey);
    g.drawText("Classic DTMF keypad tones  +  reverb", 0, 42, getWidth(), 20, juce::Justification::centred);
}

void TelephoneBeepAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(18);
    area.removeFromTop(70);

    const int btnW = 90, btnH = 58, gap = 8;
    const int startX = (getWidth() - (3 * btnW + 2 * gap)) / 2;
    const int startY = area.getY();

    for (int i = 0; i < 12; ++i) {
        const int row = i / 3;
        const int col = i % 3;
        keyButtons[i].setBounds(startX + col * (btnW + gap),
                                startY + row * (btnH + gap),
                                btnW, btnH);
    }

    int y = startY + 4 * (btnH + gap) + 25;
    reverbMixLabel.setBounds(15, y, 110, 22);
    reverbMixSlider.setBounds(130, y, getWidth() - 150, 22);

    y += 32;
    reverbSizeLabel.setBounds(15, y, 110, 22);
    reverbSizeSlider.setBounds(130, y, getWidth() - 150, 22);

    y += 32;
    toneLengthLabel.setBounds(15, y, 110, 22);
    toneLengthSlider.setBounds(130, y, getWidth() - 150, 22);
}

void TelephoneBeepAudioProcessorEditor::timerCallback()
{
    const int active = audioProcessor.currentKey.load();
    for (int i = 0; i < 12; ++i)
        keyButtons[i].setToggleState(i == active, juce::dontSendNotification);
}
