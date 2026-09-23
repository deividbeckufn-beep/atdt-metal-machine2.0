#include "PluginProcessor.h"
#include "PluginEditor.h"

MetalMachineAudioProcessor::MetalMachineAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "METALMACHINE_STATE", mm::createParameterLayout())
{
    synthParams.attach (apvts);
    synth.initialise (synthParams);
    midiLearn.setParameters (getParameters());
    masterGainParam = apvts.getRawParameterValue (mm::ParamIDs::masterGain);
    bassMonoParam   = apvts.getRawParameterValue (mm::ParamIDs::bassMono);
    presetManager   = std::make_unique<mm::PresetManager> (*this, apvts);
}

void MetalMachineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.prepare (sampleRate, samplesPerBlock);
    keyboardState.reset();
    loadMeasurer.reset (sampleRate, samplesPerBlock);
    for (auto& f : bassMonoFilters) f.prepare (sampleRate);
    lastBassMonoFreq = -1.0f;
    masterGain.reset (sampleRate, 0.02);
    masterGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (masterGainParam->load(), -59.9f));
}

bool MetalMachineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void MetalMachineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    juce::AudioProcessLoadMeasurer::ScopedTimer cpuTimer (loadMeasurer, numSamples);

    buffer.clear();
    tempo.update (getPlayHead());

    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);
    midiActivity.scan (midi);
    midiLearn.process (midi);

    synth.render (buffer, midi, numSamples, tempo);
    applyBassMono (buffer, numSamples);

    masterGain.setTargetValue (juce::Decibels::decibelsToGain (masterGainParam->load(), -59.9f));
    masterGain.applyGain (buffer, numSamples);

    peakLeft.store (buffer.getMagnitude (0, 0, numSamples));
    peakRight.store (buffer.getNumChannels() > 1 ? buffer.getMagnitude (1, 0, numSamples) : peakLeft.load());

    int voices = 0;
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (synth.getVoice (i)->isVoiceActive())
            ++voices;
    activeVoices.store (voices);
}

int MetalMachineAudioProcessor::getNumPrograms()
{
    int n = 1;   // 0 = INIT
    for (auto& p : presetManager->getPresets())
        if (p.isFactory) ++n;
    return n;
}

int MetalMachineAudioProcessor::getCurrentProgram()
{
    const int i = presetManager->getCurrentIndex();
    return (i >= 0 && presetManager->getPresets()[i].isFactory) ? presetManager->getPresets()[i].factoryIndex + 1 : 0;
}

void MetalMachineAudioProcessor::setCurrentProgram (int index)
{
    if (index <= 0)
    {
        resetToInitPatch();
        presetManager->markInit();
        return;
    }
    const auto& list = presetManager->getPresets();
    for (int i = 0; i < list.size(); ++i)
        if (list[i].isFactory && list[i].factoryIndex == index - 1)
            presetManager->load (i);
}

const juce::String MetalMachineAudioProcessor::getProgramName (int index)
{
    if (index <= 0)
        return "INIT";
    for (auto& p : presetManager->getPresets())
        if (p.isFactory && p.factoryIndex == index - 1)
            return p.category + " - " + p.name;
    return {};
}

void MetalMachineAudioProcessor::resetToInitPatch()
{
    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            rp->beginChangeGesture();
            rp->setValueNotifyingHost (rp->getDefaultValue());
            rp->endChangeGesture();
        }
}

// Bass Mono: tudo abaixo da frequencia escolhida fica mono (canal "side" passa por
// um passa-altas de 24 dB). Mantem o grave centralizado mesmo com unison/spread.
void MetalMachineAudioProcessor::applyBassMono (juce::AudioBuffer<float>& buffer, int numSamples)
{
    const float freq = bassMonoParam->load();
    if (freq < 20.0f || buffer.getNumChannels() < 2)
    {
        if (lastBassMonoFreq >= 0.0f)
            for (auto& f : bassMonoFilters) f.reset();
        lastBassMonoFreq = -1.0f;
        return;
    }

    if (freq != lastBassMonoFreq)
    {
        for (auto& f : bassMonoFilters) f.setParameters (freq, 0.3f);   // Q ~0.707 por estagio
        lastBassMonoFreq = freq;
    }

    float* l = buffer.getWritePointer (0);
    float* r = buffer.getWritePointer (1);
    for (int i = 0; i < numSamples; ++i)
    {
        const float mid  = 0.5f * (l[i] + r[i]);
        float side = 0.5f * (l[i] - r[i]);
        side = bassMonoFilters[0].process (side, mm::StateVariableFilter::Type::HighPass);
        side = bassMonoFilters[1].process (side, mm::StateVariableFilter::Type::HighPass);
        l[i] = mid + side;
        r[i] = mid - side;
    }
}

juce::String MetalMachineAudioProcessor::getMacroName (int index) const
{
    static const char* defaults[] { "AGGRESSION", "MOVEMENT", "DARKNESS", "SPACE" };
    return apvts.state.getProperty ("macroName" + juce::String (index + 1),
                                    defaults[juce::jlimit (0, 3, index)]).toString();
}

void MetalMachineAudioProcessor::setMacroName (int index, const juce::String& name)
{
    apvts.state.setProperty ("macroName" + juce::String (index + 1), name.toUpperCase().substring (0, 16), nullptr);
}

float MetalMachineAudioProcessor::getUiScale() const
{
    return (float) apvts.state.getProperty ("uiScale", 1.0f);
}

void MetalMachineAudioProcessor::setUiScale (float scale)
{
    apvts.state.setProperty ("uiScale", scale, nullptr);
}

juce::String MetalMachineAudioProcessor::getThemeName() const
{
    return apvts.state.getProperty ("theme", "METAL").toString();
}

void MetalMachineAudioProcessor::setThemeName (const juce::String& name)
{
    apvts.state.setProperty ("theme", name, nullptr);
}

juce::AudioProcessorEditor* MetalMachineAudioProcessor::createEditor()
{
    return new MetalMachineAudioProcessorEditor (*this);
}

void MetalMachineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("pluginVersion", JucePlugin_VersionString, nullptr);
    state.removeChild (state.getChildWithName ("MIDI_LEARN"), nullptr);
    state.appendChild (midiLearn.toValueTree(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void MetalMachineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            midiLearn.fromValueTree (tree.getChildWithName ("MIDI_LEARN"));
            apvts.replaceState (tree);
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetalMachineAudioProcessor();
}
