#include "PluginEditor.h"

static const char* keyLabels[12] = { "1","2","3","4","5","6","7","8","9","*","0","#" };

TelephoneBeepAudioProcessorEditor::TelephoneBeepAudioProcessorEditor(TelephoneBeepAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), piano(p)
{
    setSize(820, 720);
    setResizable(true, true);
    setResizeLimits(700, 580, 1400, 1000);

    modeBox.addItemList({"DTMF PHONE", "MOOG SYNTH", "VINTAGE DRUMS", "SLAP BASS"}, 1);
    addAndMakeVisible(modeBox);
    modeA = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "mode", modeBox);

    modeLabel.setText("ENGINE", juce::dontSendNotification);
    modeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff2222));
    modeLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    addAndMakeVisible(modeLabel);

    for (int i = 0; i < 12; ++i) {
        keyButtons[i].setButtonText(keyLabels[i]);
        keyButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a0a0a));
        keyButtons[i].setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff2222));
        keyButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        keyButtons[i].onClick = [this, i]() { audioProcessor.triggerTone(i); };
        addAndMakeVisible(keyButtons[i]);
    }

    kickBtn.setButtonText("KICK"); snareBtn.setButtonText("SNARE"); hatBtn.setButtonText("HAT"); slapBtn.setButtonText("SLAP");
    for (auto* b : {&kickBtn, &snareBtn, &hatBtn, &slapBtn}) {
        b->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a0000));
        b->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff0000));
        b->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        addAndMakeVisible(b);
    }
    kickBtn.onClick = [this]() { audioProcessor.triggerDrum(0); };
    snareBtn.onClick = [this]() { audioProcessor.triggerDrum(1); };
    hatBtn.onClick = [this]() { audioProcessor.triggerDrum(2); };
    slapBtn.onClick = [this]() { audioProcessor.triggerSlap(1.0f); };

    auto makeSlider = [&](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 16);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff3333));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffff0000));
        addAndMakeVisible(s);
    };

    makeSlider(reverbMixSlider); makeSlider(reverbSizeSlider);
    makeSlider(moogCutoffSlider); makeSlider(moogResoSlider);
    makeSlider(moogAttackSlider); makeSlider(moogDecaySlider);
    makeSlider(moogSustainSlider); makeSlider(moogReleaseSlider);
    makeSlider(kickTuneSlider); makeSlider(kickDecaySlider); makeSlider(snareSnapSlider);
    makeSlider(slapToneSlider); makeSlider(slapThumpSlider); makeSlider(slapDecaySlider);

    mixA = std::make_unique<Attachment>(audioProcessor.parameters, "reverbMix", reverbMixSlider);
    sizeA = std::make_unique<Attachment>(audioProcessor.parameters, "reverbSize", reverbSizeSlider);
    cutA = std::make_unique<Attachment>(audioProcessor.parameters, "moogCutoff", moogCutoffSlider);
    resoA = std::make_unique<Attachment>(audioProcessor.parameters, "moogReso", moogResoSlider);
    attA = std::make_unique<Attachment>(audioProcessor.parameters, "moogAttack", moogAttackSlider);
    decA = std::make_unique<Attachment>(audioProcessor.parameters, "moogDecay", moogDecaySlider);
    susA = std::make_unique<Attachment>(audioProcessor.parameters, "moogSustain", moogSustainSlider);
    relA = std::make_unique<Attachment>(audioProcessor.parameters, "moogRelease", moogReleaseSlider);
    kTuneA = std::make_unique<Attachment>(audioProcessor.parameters, "kickTune", kickTuneSlider);
    kDecA = std::make_unique<Attachment>(audioProcessor.parameters, "kickDecay", kickDecaySlider);
    sSnapA = std::make_unique<Attachment>(audioProcessor.parameters, "snareSnap", snareSnapSlider);
    slToneA = std::make_unique<Attachment>(audioProcessor.parameters, "slapTone", slapToneSlider);
    slThumpA = std::make_unique<Attachment>(audioProcessor.parameters, "slapThump", slapThumpSlider);
    slDecA = std::make_unique<Attachment>(audioProcessor.parameters, "slapDecay", slapDecaySlider);

    addAndMakeVisible(piano);

    startTimerHz(20);
    updateVisibility();
}

TelephoneBeepAudioProcessorEditor::~TelephoneBeepAudioProcessorEditor() {}

void TelephoneBeepAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0c0c0c));

    // Header bar style The Culling
    g.setColour(juce::Colour(0xff1a0000));
    g.fillRect(0, 0, getWidth(), 78);
    g.setColour(juce::Colour(0xffff0000));
    g.fillRect(0, 74, getWidth(), 4);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(34.0f, juce::Font::bold));
    g.drawText("THE CULLING", 18, 8, 400, 38, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xffff3333));
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("AUDIO WEAPON  •  DTMF  •  MOOG  •  DRUMS  •  SLAP BASS", 18, 44, 500, 18, juce::Justification::centredLeft);

    // Small logo accent
    g.setColour(juce::Colour(0xffff0000));
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("CULL", getWidth() - 120, 18, 100, 40, juce::Justification::centredRight);
}

void TelephoneBeepAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(14);
    area.removeFromTop(85);

    auto topRow = area.removeFromTop(28);
    modeLabel.setBounds(topRow.removeFromLeft(70));
    modeBox.setBounds(topRow.removeFromLeft(180));
    area.removeFromTop(8);

    auto middle = area.removeFromTop(260);
    auto keysArea = middle.removeFromLeft(260);
    int btnW = 68, btnH = 48, gap = 5;
    for (int i = 0; i < 12; ++i) {
        int row = i / 3, col = i % 3;
        keyButtons[i].setBounds(keysArea.getX() + col * (btnW + gap), keysArea.getY() + row * (btnH + gap), btnW, btnH);
    }

    auto btnArea = middle.removeFromLeft(140);
    kickBtn.setBounds(btnArea.removeFromTop(48).reduced(3));
    snareBtn.setBounds(btnArea.removeFromTop(48).reduced(3));
    hatBtn.setBounds(btnArea.removeFromTop(48).reduced(3));
    slapBtn.setBounds(btnArea.removeFromTop(48).reduced(3));

    // Knobs
    auto knobs = area.removeFromTop(120);
    int kw = 70, x = knobs.getX(), y = knobs.getY();
    reverbMixSlider.setBounds(x, y, kw, 100); x += kw + 6;
    reverbSizeSlider.setBounds(x, y, kw, 100); x += kw + 16;
    moogCutoffSlider.setBounds(x, y, kw, 100); x += kw + 6;
    moogResoSlider.setBounds(x, y, kw, 100); x += kw + 6;
    moogAttackSlider.setBounds(x, y, kw, 100); x += kw + 6;
    moogDecaySlider.setBounds(x, y, kw, 100); x += kw + 6;
    moogSustainSlider.setBounds(x, y, kw, 100); x += kw + 6;
    moogReleaseSlider.setBounds(x, y, kw, 100);

    y += 110; x = knobs.getX() + 160;
    kickTuneSlider.setBounds(x, y, kw, 100); x += kw + 6;
    kickDecaySlider.setBounds(x, y, kw, 100); x += kw + 6;
    snareSnapSlider.setBounds(x, y, kw, 100); x += kw + 16;
    slapToneSlider.setBounds(x, y, kw, 100); x += kw + 6;
    slapThumpSlider.setBounds(x, y, kw, 100); x += kw + 6;
    slapDecaySlider.setBounds(x, y, kw, 100);

    area.removeFromTop(10);
    piano.setBounds(area.removeFromBottom(90));
}

void TelephoneBeepAudioProcessorEditor::updateVisibility()
{
    int mode = modeBox.getSelectedItemIndex();
    bool dtmf = mode == 0;
    bool moog = mode == 1;
    bool drums = mode == 2;
    bool slap = mode == 3;

    for (auto& b : keyButtons) b.setVisible(dtmf);
    kickBtn.setVisible(drums);
    snareBtn.setVisible(drums);
    hatBtn.setVisible(drums);
    slapBtn.setVisible(slap);

    moogCutoffSlider.setVisible(moog);
    moogResoSlider.setVisible(moog);
    moogAttackSlider.setVisible(moog);
    moogDecaySlider.setVisible(moog);
    moogSustainSlider.setVisible(moog);
    moogReleaseSlider.setVisible(moog);

    kickTuneSlider.setVisible(drums);
    kickDecaySlider.setVisible(drums);
    snareSnapSlider.setVisible(drums);

    slapToneSlider.setVisible(slap);
    slapThumpSlider.setVisible(slap);
    slapDecaySlider.setVisible(slap);
}

void TelephoneBeepAudioProcessorEditor::timerCallback()
{
    updateVisibility();
    int active = audioProcessor.currentKey.load();
    for (int i = 0; i < 12; ++i)
        keyButtons[i].setToggleState(i == active, juce::dontSendNotification);
}
