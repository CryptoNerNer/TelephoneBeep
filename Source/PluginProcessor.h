#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

enum class EngineMode { DTMF = 0, Moog, Drums, SlapBass };

class TelephoneBeepAudioProcessor : public juce::AudioProcessor
{
public:
    TelephoneBeepAudioProcessor();
    ~TelephoneBeepAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void triggerTone(int keyIndex);
    void triggerDrum(int drum); // 0=kick, 1=snare, 2=hat
    void triggerSlap(float velocity = 1.0f);

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<int> currentKey{-1};
    std::atomic<int> currentMode{0};

private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    double currentSampleRate = 44100.0;

    // DTMF
    int samplesRemaining = 0;
    float freq1 = 697.f, freq2 = 1209.f;
    double phase1 = 0.0, phase2 = 0.0;

    // Moog synth
    double moogPhase1 = 0, moogPhase2 = 0, moogPhase3 = 0;
    float moogEnv = 0.0f;
    bool moogNoteOn = false;
    int moogNote = 60;
    float moogFilterState[4] = {};

    // Drums
    float kickEnv = 0, snareEnv = 0, hatEnv = 0;
    double kickPhase = 0, snarePhase = 0;
    float kickPitch = 120.f;

    // Slap bass
    float slapEnv = 0;
    double slapPhase = 0;
    float slapPitch = 80.f;
    float slapThump = 0;

    static constexpr float lowFreqs[4]  = {697.f, 770.f, 852.f, 941.f};
    static constexpr float highFreqs[3] = {1209.f, 1336.f, 1477.f};

    void updateReverb();
    void startTone(int key);
    void processDTMF(juce::AudioBuffer<float>& buffer, int numSamples);
    void processMoog(juce::AudioBuffer<float>& buffer, int numSamples);
    void processDrums(juce::AudioBuffer<float>& buffer, int numSamples);
    void processSlap(juce::AudioBuffer<float>& buffer, int numSamples);
    float simpleLadder(float input, float cutoff, float resonance);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TelephoneBeepAudioProcessor)
};
