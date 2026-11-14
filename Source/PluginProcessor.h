#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

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

    void triggerTone(int keyIndex); // 0=1 ... 8=9, 9=*, 10=0, 11=#

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<int> currentKey{-1};

private:
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    double currentSampleRate = 44100.0;
    int samplesRemaining = 0;
    float freq1 = 697.f, freq2 = 1209.f;
    double phase1 = 0.0, phase2 = 0.0;

    static constexpr float toneAmplitude = 0.3f;
    static constexpr float lowFreqs[4]  = {697.f, 770.f, 852.f, 941.f};
    static constexpr float highFreqs[3] = {1209.f, 1336.f, 1477.f};

    void startTone(int key);
    void updateReverb();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TelephoneBeepAudioProcessor)
};
