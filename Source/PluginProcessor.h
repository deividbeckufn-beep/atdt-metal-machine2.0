#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters/Parameters.h"
#include "Synth/SynthEngine.h"
#include "Sequencer/TempoSync.h"
#include "MIDI/MidiActivity.h"
#include "MIDI/MidiLearn.h"
#include "DSP/StateVariableFilter.h"
#include "Presets/PresetManager.h"

// =============================================================================
//  ATDT METAL MACHINE - Processor
//  MIDI do REAPER -> SynthEngine -> Master -> Saida. Salva/restaura estado.
// =============================================================================
class MetalMachineAudioProcessor : public juce::AudioProcessor,
                                   public mm::MidiLearnProvider
{
public:
    MetalMachineAudioProcessor();
    ~MetalMachineAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Presets de fabrica expostos como "programs" -> aparecem no menu de presets do REAPER
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Botao INIT: todos os parametros voltam ao padrao (com notificacao ao REAPER)
    void resetToInitPatch();

    // Preferencias de interface salvas junto com o projeto
    float getUiScale() const;
    void  setUiScale (float scale);
    juce::String getThemeName() const;
    void  setThemeName (const juce::String& name);

    mm::MidiLearn& getMidiLearn() override { return midiLearn; }

    mm::PresetManager& getPresetManager() noexcept { return *presetManager; }
    const mm::LfoEngine& getLfos() const noexcept   { return synth.getLfos(); }

    // Nomes das macros (salvos no projeto e nos presets)
    juce::String getMacroName (int index) const;
    void setMacroName (int index, const juce::String& name);

    double getCpuLoadPercent() const { return loadMeasurer.getLoadAsPercentage(); }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    mm::TempoSync tempo;
    mm::MidiActivity midiActivity;
    std::atomic<float> peakLeft { 0.0f }, peakRight { 0.0f };
    std::atomic<int> activeVoices { 0 };

private:
    mm::SynthParams synthParams;
    mm::SynthEngine synth;
    std::atomic<float>* masterGainParam = nullptr;
    juce::SmoothedValue<float> masterGain;
    juce::AudioProcessLoadMeasurer loadMeasurer;

    void applyBassMono (juce::AudioBuffer<float>&, int numSamples);
    std::atomic<float>* bassMonoParam = nullptr;
    mm::StateVariableFilter bassMonoFilters[2];
    float lastBassMonoFreq = -1.0f;

    std::unique_ptr<mm::PresetManager> presetManager;
    mm::MidiLearn midiLearn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetalMachineAudioProcessor)
};
