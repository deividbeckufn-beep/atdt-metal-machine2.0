#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// =============================================================================
//  Parametros do ATDT METAL MACHINE (todos automatizaveis no REAPER).
//  Regra: nunca renomear um ID ja publicado. Apenas ADICIONAR novos.
// =============================================================================
namespace mm
{
namespace ParamIDs
{
    inline juce::String osc (int index, const char* name) { return "osc" + juce::String (index) + "_" + name; }

    inline constexpr const char* subWave      = "sub_wave";
    inline constexpr const char* subOctave    = "sub_octave";
    inline constexpr const char* subLevel     = "sub_level";

    inline constexpr const char* noiseType    = "noise_type";
    inline constexpr const char* noiseLevel   = "noise_level";

    inline constexpr const char* filterType   = "filter_type";
    inline constexpr const char* cutoff       = "filter_cutoff";
    inline constexpr const char* resonance    = "filter_resonance";
    inline constexpr const char* filterDrive  = "filter_drive";
    inline constexpr const char* keyTrack     = "filter_keytrack";
    inline constexpr const char* filterMix    = "filter_mix";
    inline constexpr const char* filterEnvAmt = "filter_env_amount";

    inline constexpr const char* fenvAttack   = "fenv_attack";
    inline constexpr const char* fenvDecay    = "fenv_decay";
    inline constexpr const char* fenvSustain  = "fenv_sustain";
    inline constexpr const char* fenvRelease  = "fenv_release";

    inline constexpr const char* ampAttack    = "amp_attack";
    inline constexpr const char* ampDecay     = "amp_decay";
    inline constexpr const char* ampSustain   = "amp_sustain";
    inline constexpr const char* ampRelease   = "amp_release";

    inline constexpr const char* masterGain   = "master_gain";
    inline constexpr const char* bendRange    = "pitch_bend_range";

    inline constexpr const char* voiceMode    = "voice_mode";
    inline constexpr const char* voiceCount   = "voice_count";
    inline constexpr const char* glide        = "voice_glide";
    inline constexpr const char* unisonVoices = "unison_voices";
    inline constexpr const char* unisonDetune = "unison_detune";
    inline constexpr const char* unisonSpread = "unison_spread";
    inline constexpr const char* bassMono     = "bass_mono";

    inline juce::String lfo (int index, const char* name)  { return "lfo" + juce::String (index) + "_" + name; }
    inline juce::String mod (int slot, const char* name)   { return "mod" + juce::String (slot) + "_" + name; }
    inline juce::String macro (int index)                  { return "macro" + juce::String (index); }
}

inline const juce::StringArray voiceModeNames  { "Poly", "Mono", "Legato" };
inline const juce::StringArray voiceCountNames { "1", "2", "4", "8", "16", "32" };
inline constexpr int voiceCountValues[] { 1, 2, 4, 8, 16, 32 };

inline const juce::StringArray lfoShapeNames { "Sine", "Triangle", "Saw", "Square", "S&H", "Random" };
inline const juce::StringArray lfoDivNames   { "2/1", "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T" };
inline constexpr double lfoDivBeats[] { 8.0, 4.0, 2.0, 1.0, 0.5, 0.25, 0.125, 2.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0 };

inline const juce::StringArray waveNames    { "Sine", "Triangle", "Saw", "Square", "Pulse", "Noise" };
inline const juce::StringArray subWaveNames { "Sine", "Triangle", "Square" };
inline const juce::StringArray noiseNames   { "White", "Pink", "Digital" };
inline const juce::StringArray filterNames  { "LP12", "LP24", "HP12", "HP24", "BP", "NOTCH" };

inline constexpr int numOscillators = 3;

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
} // namespace mm
