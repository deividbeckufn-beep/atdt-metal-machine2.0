#pragma once
#include <juce_graphics/juce_graphics.h>

namespace mm
{
// =============================================================================
//  Sistema de temas. METAL = tema oficial (preto + vermelho neon).
//  Para criar um tema novo: copie um bloco abaixo e registre em all().
// =============================================================================
struct Theme
{
    juce::String name;
    juce::Colour background, panel, panelRaised, edge, track, display,
                 accent, accentSoft, accentDark, text, textDim;

    static const Theme& metal()
    {
        static const Theme t { "METAL",
            juce::Colour (0xff050506), juce::Colour (0xff0e0f12), juce::Colour (0xff16181c),
            juce::Colour (0xff2a1519), juce::Colour (0xff25272c), juce::Colour (0xff08090b),
            juce::Colour (0xffff1f3d), juce::Colour (0xffff5a6e), juce::Colour (0xff6e0c18),
            juce::Colour (0xffe6e8ec), juce::Colour (0xff80868f) };
        return t;
    }

    static const Theme& cyber()
    {
        static const Theme t { "CYBER",
            juce::Colour (0xff04070a), juce::Colour (0xff0c1116), juce::Colour (0xff131a21),
            juce::Colour (0xff16303a), juce::Colour (0xff222a31), juce::Colour (0xff060a0d),
            juce::Colour (0xff00d9ff), juce::Colour (0xff66e8ff), juce::Colour (0xff04506a),
            juce::Colour (0xffe4eef2), juce::Colour (0xff7a8a94) };
        return t;
    }

    static juce::Array<const Theme*> all() { return { &metal(), &cyber() }; }

    static const Theme& byName (const juce::String& n)
    {
        for (auto* t : all())
            if (t->name == n)
                return *t;
        return metal();
    }
};
} // namespace mm
