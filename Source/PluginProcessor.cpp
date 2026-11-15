#include "PluginProcessor.h"
#include "PluginEditor.h"

TelephoneBeepAudioProcessor::TelephoneBeepAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("PARAMS"), {
          std::make_unique<juce::AudioParameterChoice>("mode", "Mode", juce::StringArray{"DTMF", "MOOG", "DRUMS", "SLAP"}, 0),
          std::make_unique<juce::AudioParameterFloat>("reverbMix", "Reverb Mix", 0.0f, 1.0f, 0.2f),
          std::make_unique<juce::AudioParameterFloat>("reverbSize", "Reverb Size", 0.0f, 1.0f, 0.4f),
          // Moog
          std::make_unique<juce::AudioParameterFloat>("moogCutoff", "Moog Cutoff", 20.0f, 18000.0f, 800.0f),
          std::make_unique<juce::AudioParameterFloat>("moogReso", "Moog Resonance", 0.0f, 0.95f, 0.3f),
          std::make_unique<juce::AudioParameterFloat>("moogAttack", "Moog Attack", 0.001f, 2.0f, 0.01f),
          std::make_unique<juce::AudioParameterFloat>("moogDecay", "Moog Decay", 0.01f, 3.0f, 0.3f),
          std::make_unique<juce::AudioParameterFloat>("moogSustain", "Moog Sustain", 0.0f, 1.0f, 0.6f),
          std::make_unique<juce::AudioParameterFloat>("moogRelease", "Moog Release", 0.01f, 4.0f, 0.4f),
          // Drums
          std::make_unique<juce::AudioParameterFloat>("kickTune", "Kick Tune", 40.0f, 200.0f, 80.0f),
          std::make_unique<juce::AudioParameterFloat>("kickDecay", "Kick Decay", 0.05f, 1.5f, 0.4f),
          std::make_unique<juce::AudioParameterFloat>("snareSnap", "Snare Snap", 0.0f, 1.0f, 0.6f),
          // Slap
          std::make_unique<juce::AudioParameterFloat>("slapTone", "Slap Tone", 40.0f, 200.0f, 70.0f),
          std::make_unique<juce::AudioParameterFloat>("slapThump", "Slap Thump", 0.0f, 1.0f, 0.5f),
          std::make_unique<juce::AudioParameterFloat>("slapDecay", "Slap Decay", 0.05f, 1.0f, 0.25f)
      })
{
}

TelephoneBeepAudioProcessor::~TelephoneBeepAudioProcessor() {}

void TelephoneBeepAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(getTotalNumOutputChannels()) };
    reverb.prepare(spec);
    updateReverb();
    std::fill(std::begin(moogFilterState), std::end(moogFilterState), 0.0f);
}

void TelephoneBeepAudioProcessor::releaseResources() {}

void TelephoneBeepAudioProcessor::updateReverb()
{
    reverbParams.roomSize = *parameters.getRawParameterValue("reverbSize");
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = *parameters.getRawParameterValue("reverbMix");
    reverbParams.dryLevel = 1.0f - reverbParams.wetLevel * 0.5f;
    reverbParams.width = 0.9f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);
}

float TelephoneBeepAudioProcessor::simpleLadder(float input, float cutoff, float resonance)
{
    float g = std::tan(juce::MathConstants<float>::pi * cutoff / static_cast<float>(currentSampleRate));
    float k = resonance * 3.5f;
    float a = g / (1.0f + g);

    float feedback = k * moogFilterState[3];
    float u = (input - feedback) / (1.0f + k * a * a * a * a);

    for (int i = 0; i < 4; ++i) {
        float v = a * (u - moogFilterState[i]);
        u = v + moogFilterState[i];
        moogFilterState[i] = u + v;
    }
    return u;
}

void TelephoneBeepAudioProcessor::startTone(int key)
{
    if (key < 0 || key > 11) return;
    int row, col;
    if (key <= 8) { row = key / 3; col = key % 3; }
    else if (key == 9) { row = 3; col = 0; }
    else if (key == 10) { row = 3; col = 1; }
    else { row = 3; col = 2; }
    freq1 = lowFreqs[row];
    freq2 = highFreqs[col];
    phase1 = phase2 = 0.0;
    float lengthMs = 150.0f;
    samplesRemaining = static_cast<int>(currentSampleRate * lengthMs / 1000.0f);
    currentKey.store(key);
}

void TelephoneBeepAudioProcessor::triggerTone(int keyIndex) { startTone(keyIndex); }

void TelephoneBeepAudioProcessor::triggerDrum(int drum)
{
    if (drum == 0) { kickEnv = 1.0f; kickPhase = 0; kickPitch = *parameters.getRawParameterValue("kickTune") * 1.8f; }
    else if (drum == 1) { snareEnv = 1.0f; snarePhase = 0; }
    else if (drum == 2) { hatEnv = 1.0f; }
}

void TelephoneBeepAudioProcessor::triggerSlap(float velocity)
{
    slapEnv = velocity;
    slapThump = *parameters.getRawParameterValue("slapThump") * velocity;
    slapPhase = 0;
    slapPitch = *parameters.getRawParameterValue("slapTone");
}

void TelephoneBeepAudioProcessor::processDTMF(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (samplesRemaining <= 0) return;
    const int numCh = buffer.getNumChannels();
    const double twoPi = juce::MathConstants<double>::twoPi;
    const double sr = currentSampleRate;
    const int samplesToDo = std::min(numSamples, samplesRemaining);

    for (int ch = 0; ch < numCh; ++ch) {
        auto* data = buffer.getWritePointer(ch);
        double p1 = phase1, p2 = phase2;
        for (int i = 0; i < samplesToDo; ++i) {
            float s = (std::sin(p1) + std::sin(p2)) * 0.25f;
            data[i] += s;
            p1 += twoPi * freq1 / sr;
            p2 += twoPi * freq2 / sr;
            if (p1 >= twoPi) p1 -= twoPi;
            if (p2 >= twoPi) p2 -= twoPi;
        }
        if (ch == numCh - 1) { phase1 = p1; phase2 = p2; }
    }
    samplesRemaining -= samplesToDo;
    if (samplesRemaining <= 0) currentKey.store(-1);
}

void TelephoneBeepAudioProcessor::processMoog(juce::AudioBuffer<float>& buffer, int numSamples)
{
    const int numCh = buffer.getNumChannels();
    const double twoPi = juce::MathConstants<double>::twoPi;
    const double sr = currentSampleRate;
    float cutoff = *parameters.getRawParameterValue("moogCutoff");
    float reso = *parameters.getRawParameterValue("moogReso");

    float attack = *parameters.getRawParameterValue("moogAttack");
    float decay = *parameters.getRawParameterValue("moogDecay");
    float sustain = *parameters.getRawParameterValue("moogSustain");
    float release = *parameters.getRawParameterValue("moogRelease");

    float envInc = 0.0f;
    if (moogNoteOn) {
        if (moogEnv < 1.0f) envInc = 1.0f / (attack * static_cast<float>(sr) + 1.0f);
        else envInc = -(1.0f - sustain) / (decay * static_cast<float>(sr) + 1.0f);
    } else {
        envInc = -moogEnv / (release * static_cast<float>(sr) + 1.0f);
    }

    float freq = 440.0f * std::pow(2.0f, (moogNote - 69) / 12.0f);

    for (int ch = 0; ch < numCh; ++ch) {
        auto* data = buffer.getWritePointer(ch);
        double p1 = moogPhase1, p2 = moogPhase2, p3 = moogPhase3;
        for (int i = 0; i < numSamples; ++i) {
            moogEnv = juce::jlimit(0.0f, 1.0f, moogEnv + envInc);
            float osc = (std::sin(p1) + std::sin(p2 * 1.01) * 0.6f + std::sin(p3 * 0.5) * 0.4f) * 0.3f;
            float filtered = simpleLadder(osc * moogEnv, cutoff, reso);
            data[i] += filtered * moogEnv * 0.6f;
            p1 += twoPi * freq / sr;
            p2 += twoPi * freq * 1.005 / sr;
            p3 += twoPi * freq * 0.5 / sr;
            if (p1 >= twoPi) p1 -= twoPi;
            if (p2 >= twoPi) p2 -= twoPi;
            if (p3 >= twoPi) p3 -= twoPi;
        }
        if (ch == numCh - 1) { moogPhase1 = p1; moogPhase2 = p2; moogPhase3 = p3; }
    }
}

void TelephoneBeepAudioProcessor::processDrums(juce::AudioBuffer<float>& buffer, int numSamples)
{
    const int numCh = buffer.getNumChannels();
    const double twoPi = juce::MathConstants<double>::twoPi;
    const double sr = currentSampleRate;
    float kickDec = *parameters.getRawParameterValue("kickDecay");
    float snap = *parameters.getRawParameterValue("snareSnap");

    for (int ch = 0; ch < numCh; ++ch) {
        auto* data = buffer.getWritePointer(ch);
        double kp = kickPhase, sp = snarePhase;
        float ke = kickEnv, se = snareEnv, he = hatEnv;
        float kpitch = kickPitch;

        for (int i = 0; i < numSamples; ++i) {
            // Kick
            float kick = 0;
            if (ke > 0.001f) {
                kick = std::sin(kp) * ke;
                kp += twoPi * kpitch / sr;
                if (kp >= twoPi) kp -= twoPi;
                kpitch *= 0.9995f;
                ke *= std::exp(-1.0f / (kickDec * static_cast<float>(sr)));
            }
            // Snare
            float snare = 0;
            if (se > 0.001f) {
                float tone = std::sin(sp) * 0.4f;
                float noise = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * snap;
                snare = (tone + noise) * se;
                sp += twoPi * 180.0 / sr;
                if (sp >= twoPi) sp -= twoPi;
                se *= 0.9992f;
            }
            // Hat
            float hat = 0;
            if (he > 0.001f) {
                hat = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * he * 0.3f;
                he *= 0.98f;
            }
            data[i] += kick * 0.7f + snare * 0.5f + hat * 0.4f;
        }
        if (ch == numCh - 1) {
            kickPhase = kp; snarePhase = sp;
            kickEnv = ke; snareEnv = se; hatEnv = he;
            kickPitch = kpitch;
        }
    }
}

void TelephoneBeepAudioProcessor::processSlap(juce::AudioBuffer<float>& buffer, int numSamples)
{
    const int numCh = buffer.getNumChannels();
    const double twoPi = juce::MathConstants<double>::twoPi;
    const double sr = currentSampleRate;
    float dec = *parameters.getRawParameterValue("slapDecay");

    for (int ch = 0; ch < numCh; ++ch) {
        auto* data = buffer.getWritePointer(ch);
        double ph = slapPhase;
        float env = slapEnv;
        float thump = slapThump;
        float pitch = slapPitch;

        for (int i = 0; i < numSamples; ++i) {
            if (env > 0.001f || thump > 0.001f) {
                float body = std::sin(ph) * env * 0.6f;
                float click = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * thump * 0.3f;
                data[i] += body + click;
                ph += twoPi * pitch / sr;
                if (ph >= twoPi) ph -= twoPi;
                env *= std::exp(-1.0f / (dec * static_cast<float>(sr)));
                thump *= 0.97f;
            }
        }
        if (ch == numCh - 1) {
            slapPhase = ph;
            slapEnv = env;
            slapThump = thump;
        }
    }
}

void TelephoneBeepAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    updateReverb();
    buffer.clear();

    int mode = static_cast<int>(*parameters.getRawParameterValue("mode"));
    currentMode.store(mode);

    for (const auto metadata : midi) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            int note = msg.getNoteNumber();
            if (mode == 0 && note >= 60 && note <= 71) startTone(note - 60);
            else if (mode == 1) { moogNote = note; moogNoteOn = true; moogEnv = 0; }
            else if (mode == 2) {
                if (note == 36 || note == 35) triggerDrum(0);
                else if (note == 38 || note == 40) triggerDrum(1);
                else if (note == 42 || note == 44) triggerDrum(2);
            }
            else if (mode == 3) triggerSlap(msg.getVelocity() / 127.0f);
        }
        else if (msg.isNoteOff() && mode == 1) moogNoteOn = false;
    }

    const int numSamples = buffer.getNumSamples();
    if (mode == 0) processDTMF(buffer, numSamples);
    else if (mode == 1) processMoog(buffer, numSamples);
    else if (mode == 2) processDrums(buffer, numSamples);
    else if (mode == 3) processSlap(buffer, numSamples);

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
