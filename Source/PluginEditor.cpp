#include "PluginEditor.h"

static const char* keyLabels[12] = { "1","2","3","4","5","6","7","8","9","*","0","#" };

TelephoneBeepAudioProcessorEditor::TelephoneBeepAudioProcessorEditor(TelephoneBeepAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(720, 640);
    setResizable(true, true);
    setResizeLimits(600, 500, 1200, 900);

    modeBox.addItemList({"DTMF PHONE", "MOOG SYNTH", "VINTAGE DRUMS", "SLAP BASS"}, 1);
    addAndMakeVisible(modeBox);
    modeA = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.parameters, "mode", modeBox);

    modeLabel.setText("ENGINE", juce::dontSendNotification);
    modeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff2222));
    modeLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    addAndMakeVisible(modeLabel);

    for (int i = 0; i < 12; ++i) {
        keyButtons[i].setButtonText(keyLabels[i]);
        keyButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1a1a));
        keyButtons[i].setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff3333));
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

    auto makeSlider = [&](juce::Slider& s, const juce::String& id) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff3333));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffff0000));
        addAndMakeVisible(s);
    };

    makeSlider(reverbMixSlider, "reverbMix");
    makeSlider(reverbSizeSlider, "reverbSize");
    makeSlider(moogCutoffSlider, "moogCutoff");
    makeSlider(moogResoSlider, "moogReso");
    makeSlider(moogAttackSlider, "moogAttack");
    makeSlider(moogDecaySlider, "moogDecay");
    makeSlider(moogSustainSlider, "moogSustain");
    makeSlider(moogReleaseSlider, "moogRelease");
    makeSlider(kickTuneSlider, "kickTune");
    makeSlider(kickDecaySlider, "kickDecay");
    makeSlider(snareSnapSlider, "snareSnap");
    makeSlider(slapToneSlider, "slapTone");
    makeSlider(slapThumpSlider, "slapThump");
    makeSlider(slapDecaySlider, "slapDecay");

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

    startTimerHz(20);
    updateVisibility();
}

TelephoneBeepAudioProcessorEditor::~TelephoneBeepAudioProcessorEditor() {}

void TelephoneBeepAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0a0a0a));

    // Blood drip style header
    g.setColour(juce::Colour(0xffaa0000));
    g.fillRect(0, 0, getWidth(), 70);
    g.setColour(juce::Colours::black);
    g.fillRect(0, 65, getWidth(), 5);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    g.drawText("THE CULLING", 20, 10, getWidth() - 40, 40, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xffff3333));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText("AUDIO WEAPON  //  DTMF  •  MOOG  •  DRUMS  •  SLAP", 20, 42, getWidth() - 40, 20, juce::Justification::centredLeft);
}

void TelephoneBeepAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(16);
    area.removeFromTop(80);

    modeLabel.setBounds(area.removeFromTop(20).removeFromLeft(80));
    modeBox.setBounds(area.removeFromTop(30).removeFromLeft(200));
    area.removeFromTop(10);

    auto top = area.removeFromTop(280);
    // Keypad left
    auto keys = top.removeFromLeft(280);
    int btnW = 70, btnH = 50, gap = 6;
    for (int i = 0; i < 12; ++i) {
        int row = i / 3, col = i % 3;
        keyButtons[i].setBounds(keys.getX() + col * (btnW + gap), keys.getY() + row * (btnH + gap), btnW, btnH);
    }

    // Drum / slap buttons
    auto buttons = top.removeFromLeft(200);
    kickBtn.setBounds(buttons.removeFromTop(50).reduced(4));
    snareBtn.setBounds(buttons.removeFromTop(50).reduced(4));
    hatBtn.setBounds(buttons.removeFromTop(50).reduced(4));
    slapBtn.setBounds(buttons.removeFromTop(50).reduced(4));

    // Controls
    auto controls = area;
    int knobW = 80;
    int x = controls.getX();
    int y = controls.getY();

    reverbMixSlider.setBounds(x, y, knobW, 90); x += knobW + 10;
    reverbSizeSlider.setBounds(x, y, knobW, 90); x += knobW + 20;

    moogCutoffSlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    moogResoSlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    moogAttackSlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    moogDecaySlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    moogSustainSlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    moogReleaseSlider.setBounds(x, y, knobW, 90);

    y += 110; x = controls.getX() + 180;
    kickTuneSlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    kickDecaySlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    snareSnapSlider.setBounds(x, y, knobW, 90); x += knobW + 20;

    slapToneSlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    slapThumpSlider.setBounds(x, y, knobW, 90); x += knobW + 8;
    slapDecaySlider.setBounds(x, y, knobW, 90);
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
