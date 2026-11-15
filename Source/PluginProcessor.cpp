#include "PluginProcessor.h"
#include "PluginEditor.h"

TelephoneBeepAudioProcessor::TelephoneBeepAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("PARAMS"), {
          std::make_unique<juce::AudioParameterFloat>("reverbMix", "Reverb Mix", 0.0f, 1.0f, 0.25f),
          std::make_unique<juce::AudioParameterFloat>("reverbSize", "Reverb Size", 0.0f, 1.0f, 0.45f),
          std::make_unique<juce::AudioParameterFloat>("toneLength", "Tone Length (ms)", 60.0f, 400.0f, 130.0f)
      })
{
}

TelephoneBeepAudioProcessor::~TelephoneBeepAudioProcessor() {}

void TelephoneBeepAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec{ sampleRate,
                                 static_cast<juce::uint32>(samplesPerBlock),
                                 static_cast<juce::uint32>(getTotalNumOutputChannels()) };
    reverb.prepare(spec);
    updateReverb();
}

void TelephoneBeepAudioProcessor::releaseResources() {}

void TelephoneBeepAudioProcessor::updateReverb()
{
    reverbParams.roomSize   = *parameters.getRawParameterValue("reverbSize");
    reverbParams.damping    = 0.5f;
    reverbParams.wetLevel   = *parameters.getRawParameterValue("reverbMix");
    reverbParams.dryLevel   = 1.0f - reverbParams.wetLevel * 0.55f;
    reverbParams.width      = 0.95f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);
}

void TelephoneBeepAudioProcessor::startTone(int key)
{
    if (key < 0 || key > 11) return;

    int row, col;
    if (key <= 8) { row = key / 3; col = key % 3; }
    else if (key == 9)  { row = 3; col = 0; }
    else if (key == 10) { row = 3; col = 1; }
    else                { row = 3; col = 2; }

    freq1 = lowFreqs[row];
    freq2 = highFreqs[col];
    phase1 = phase2 = 0.0;

    float lengthMs = *parameters.getRawParameterValue("toneLength");
    samplesRemaining = static_cast<int>(currentSampleRate * lengthMs / 1000.0f);
    currentKey.store(key);
}

void TelephoneBeepAudioProcessor::triggerTone(int keyIndex) { startTone(keyIndex); }

void TelephoneBeepAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    updateReverb();
    buffer.clear();

    for (const auto metadata : midi) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            const int note = msg.getNoteNumber();
            if (note >= 60 && note <= 71)
                startTone(note - 60);
        }
    }

    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    const double twoPi = juce::MathConstants<double>::twoPi;
    const double sr = currentSampleRate;

    if (samplesRemaining > 0) {
        const int samplesToDo = std::min(numSamples, samplesRemaining);
        for (int ch = 0; ch < numCh; ++ch) {
            auto* data = buffer.getWritePointer(ch);
            double p1 = phase1, p2 = phase2;
            for (int i = 0; i < samplesToDo; ++i) {
                const float s1 = static_cast<float>(std::sin(p1));
                const float s2 = static_cast<float>(std::sin(p2));
                const float env = (i < 10) ? (static_cast<float>(i) / 10.0f) : 1.0f;
                data[i] = (s1 + s2) * 0.5f * toneAmplitude * env;
                p1 += twoPi * freq1 / sr;
                p2 += twoPi * freq2 / sr;
                if (p1 >= twoPi) p1 -= twoPi;
                if (p2 >= twoPi) p2 -= twoPi;
            }
            if (ch == numCh - 1) { phase1 = p1; phase2 = p2; }
        }
        samplesRemaining -= samplesToDo;
        if (samplesRemaining <= 0)
            currentKey.store(-1);
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

juce::AudioProcessorEditor* TelephoneBeepAudioProcessor::createEditor()
{
    return new TelephoneBeepAudioProcessorEditor(*this);
}

void TelephoneBeepAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TelephoneBeepAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TelephoneBeepAudioProcessor();
}
