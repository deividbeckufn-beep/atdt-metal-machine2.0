#include "Parameters.h"
#include "Synth/ModulationDefs.h"

using namespace juce;

namespace mm
{
namespace
{
    using Attr = AudioParameterFloatAttributes;

    String formatTime (float seconds, int)
    {
        return seconds < 1.0f ? String (seconds * 1000.0f, 0) + " ms" : String (seconds, 2) + " s";
    }

    std::unique_ptr<AudioParameterFloat> timeParam (const String& id, const String& name, float def)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name,
            NormalisableRange<float> (0.001f, 10.0f, 0.0f, 0.3f), def,
            Attr().withStringFromValueFunction (formatTime));
    }

    std::unique_ptr<AudioParameterFloat> percentParam (const String& id, const String& name,
                                                       float min, float max, float def)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name,
            NormalisableRange<float> (min, max), def,
            Attr().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + "%"; }));
    }
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    // ---------------------------------------------------------------- OSCs
    // INIT: OSC1 Saw + OSC2 Saw desafinado; OSC3 desligado; SUB seno -1 oitava
    const int   defWave[]  { 2, 2, 3 };
    const float defFine[]  { 0.0f, 9.0f, 0.0f };
    const float defLevel[] { 0.75f, 0.6f, 0.0f };
    const float defPan[]   { -0.25f, 0.25f, 0.0f };

    for (int i = 1; i <= numOscillators; ++i)
    {
        const String p = "OSC" + String (i) + " ";
        auto g = std::make_unique<AudioProcessorParameterGroup> ("osc" + String (i), "OSC " + String (i), "|");

        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::osc (i, "wave"), 1 }, p + "Wave", waveNames, defWave[i - 1]));
        g->addChild (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::osc (i, "octave"), 1 }, p + "Octave", -3, 3, 0));
        g->addChild (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::osc (i, "semi"), 1 }, p + "Semitone", -12, 12, 0));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::osc (i, "fine"), 1 }, p + "Fine",
            NormalisableRange<float> (-100.0f, 100.0f, 0.1f), defFine[i - 1],
            Attr().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " ct"; })));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::osc (i, "phase"), 1 }, p + "Phase",
            NormalisableRange<float> (0.0f, 1.0f), 0.0f,
            Attr().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 360.0f)) + "deg"; })));
        g->addChild (percentParam (ParamIDs::osc (i, "level"), p + "Level", 0.0f, 1.0f, defLevel[i - 1]));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::osc (i, "pan"), 1 }, p + "Pan",
            NormalisableRange<float> (-1.0f, 1.0f), defPan[i - 1],
            Attr().withStringFromValueFunction ([] (float v, int)
            {
                if (std::abs (v) < 0.01f) return String ("C");
                return String (roundToInt (std::abs (v) * 100.0f)) + (v < 0 ? " L" : " R");
            })));
        layout.add (std::move (g));
    }

    // ----------------------------------------------------------- SUB / NOISE
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> ("sub", "SUB", "|");
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::subWave, 1 }, "Sub Wave", subWaveNames, 0));
        g->addChild (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::subOctave, 1 }, "Sub Octave", -2, 0, -1));
        g->addChild (percentParam (ParamIDs::subLevel, "Sub Level", 0.0f, 1.0f, 0.5f));
        layout.add (std::move (g));
    }
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> ("noise", "NOISE", "|");
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::noiseType, 1 }, "Noise Type", noiseNames, 0));
        g->addChild (percentParam (ParamIDs::noiseLevel, "Noise Level", 0.0f, 1.0f, 0.0f));
        layout.add (std::move (g));
    }

    // -------------------------------------------------------------- FILTER
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> ("filter", "FILTER", "|");
        NormalisableRange<float> cutoffRange (20.0f, 20000.0f);
        cutoffRange.setSkewForCentre (1000.0f);

        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::filterType, 1 }, "Filter Type", filterNames, 1));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::cutoff, 1 }, "Filter Cutoff", cutoffRange, 2200.0f,
            Attr().withStringFromValueFunction ([] (float v, int)
            {
                return v < 1000.0f ? String (roundToInt (v)) + " Hz" : String (v / 1000.0f, 2) + " kHz";
            })));
        g->addChild (percentParam (ParamIDs::resonance,    "Filter Resonance",  0.0f, 1.0f, 0.25f));
        g->addChild (percentParam (ParamIDs::filterDrive,  "Filter Drive",      0.0f, 1.0f, 0.2f));
        g->addChild (percentParam (ParamIDs::keyTrack,     "Filter Keytrack",   0.0f, 1.0f, 0.3f));
        g->addChild (percentParam (ParamIDs::filterMix,    "Filter Mix",        0.0f, 1.0f, 1.0f));
        g->addChild (percentParam (ParamIDs::filterEnvAmt, "Filter Env Amount", -1.0f, 1.0f, 0.35f));

        g->addChild (timeParam (ParamIDs::fenvAttack, "Filter Env Attack", 0.003f));
        g->addChild (timeParam (ParamIDs::fenvDecay,  "Filter Env Decay",  0.25f));
        g->addChild (percentParam (ParamIDs::fenvSustain, "Filter Env Sustain", 0.0f, 1.0f, 0.25f));
        g->addChild (timeParam (ParamIDs::fenvRelease, "Filter Env Release", 0.3f));
        layout.add (std::move (g));
    }

    // ----------------------------------------------------------------- AMP
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> ("amp", "AMP", "|");
        g->addChild (timeParam (ParamIDs::ampAttack, "Amp Attack", 0.002f));
        g->addChild (timeParam (ParamIDs::ampDecay,  "Amp Decay",  0.2f));
        g->addChild (percentParam (ParamIDs::ampSustain, "Amp Sustain", 0.0f, 1.0f, 0.85f));
        g->addChild (timeParam (ParamIDs::ampRelease, "Amp Release", 0.2f));
        layout.add (std::move (g));
    }

    // -------------------------------------------------------------- MASTER
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> ("master", "MASTER", "|");
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::masterGain, 1 }, "Master Gain",
            NormalisableRange<float> (-60.0f, 18.0f, 0.1f), -6.0f,
            Attr().withStringFromValueFunction ([] (float v, int) { return v <= -59.9f ? String ("-inf") : String (v, 1) + " dB"; })));
        g->addChild (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::bendRange, 1 }, "Pitch Bend Range", 1, 24, 2));
        layout.add (std::move (g));
    }

    // --------------------------------------------------------------- VOICE
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> ("voice", "VOICE", "|");
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::voiceMode, 1 }, "Voice Mode", voiceModeNames, 0));
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::voiceCount, 1 }, "Voices", voiceCountNames, 4));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::glide, 1 }, "Glide",
            NormalisableRange<float> (0.0f, 2.0f, 0.0f, 0.35f), 0.0f,
            Attr().withStringFromValueFunction ([] (float v, int)
            {
                return v < 0.0005f ? String ("OFF") : formatTime (v, 0);
            })));
        g->addChild (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::unisonVoices, 1 }, "Unison", 1, 8, 1));
        g->addChild (percentParam (ParamIDs::unisonDetune, "Unison Detune", 0.0f, 1.0f, 0.25f));
        g->addChild (percentParam (ParamIDs::unisonSpread, "Unison Spread", 0.0f, 1.0f, 0.6f));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::bassMono, 1 }, "Bass Mono",
            NormalisableRange<float> (0.0f, 300.0f, 1.0f), 0.0f,
            Attr().withStringFromValueFunction ([] (float v, int)
            {
                return v < 20.0f ? String ("OFF") : String (roundToInt (v)) + " Hz";
            })));
        layout.add (std::move (g));
    }

    // ---------------------------------------------------------------- LFOs
    for (int i = 1; i <= numLfos; ++i)
    {
        const String p = "LFO" + String (i) + " ";
        auto g = std::make_unique<AudioProcessorParameterGroup> ("lfo" + String (i), "LFO " + String (i), "|");
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::lfo (i, "shape"), 1 }, p + "Shape", lfoShapeNames, 0));
        g->addChild (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::lfo (i, "sync"), 1 }, p + "Sync", true));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::lfo (i, "rate"), 1 }, p + "Rate",
            NormalisableRange<float> (0.02f, 40.0f, 0.0f, 0.3f), 2.0f,
            Attr().withStringFromValueFunction ([] (float v, int) { return String (v, v < 10.0f ? 2 : 1) + " Hz"; })));
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::lfo (i, "div"), 1 }, p + "Division", lfoDivNames, 3));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::lfo (i, "phase"), 1 }, p + "Phase",
            NormalisableRange<float> (0.0f, 1.0f), 0.0f,
            Attr().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 360.0f)) + "deg"; })));
        g->addChild (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::lfo (i, "fade"), 1 }, p + "Fade In",
            NormalisableRange<float> (0.0f, 5.0f, 0.0f, 0.4f), 0.0f,
            Attr().withStringFromValueFunction ([] (float v, int) { return v < 0.0005f ? String ("OFF") : formatTime (v, 0); })));
        g->addChild (percentParam (ParamIDs::lfo (i, "amount"), p + "Amount", 0.0f, 1.0f, 1.0f));
        layout.add (std::move (g));
    }

    // ------------------------------------------------------ MOD MATRIX (8)
    for (int i = 1; i <= numModSlots; ++i)
    {
        const String p = "Mod " + String (i) + " ";
        auto g = std::make_unique<AudioProcessorParameterGroup> ("mod" + String (i), "MOD SLOT " + String (i), "|");
        // INIT: slot 1 = Mod Wheel -> Cutoff (mesmo comportamento da Fase 1)
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::mod (i, "src"), 1 }, p + "Source",
                                                             modSourceNames(), i == 1 ? (int) ModSource::ModWheel : 0));
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::mod (i, "dst"), 1 }, p + "Destination",
                                                             modDestNames(), i == 1 ? (int) ModDest::Cutoff : 0));
        g->addChild (percentParam (ParamIDs::mod (i, "amt"), p + "Amount", -1.0f, 1.0f, i == 1 ? 0.35f : 0.5f));
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::mod (i, "curve"), 1 }, p + "Curve", modCurveNames(), 0));
        g->addChild (std::make_unique<AudioParameterChoice> (ParameterID { ParamIDs::mod (i, "mode"), 1 }, p + "Mode", modModeNames(), 0));
        layout.add (std::move (g));
    }

    // -------------------------------------------------------------- MACROS
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> ("macros", "MACROS", "|");
        for (int i = 1; i <= numMacros; ++i)
            g->addChild (percentParam (ParamIDs::macro (i), "Macro " + String (i), 0.0f, 1.0f, 0.0f));
        layout.add (std::move (g));
    }

    return layout;
}
} // namespace mm
