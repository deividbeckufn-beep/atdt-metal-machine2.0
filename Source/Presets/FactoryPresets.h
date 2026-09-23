#pragma once
#include <vector>
#include <utility>
#include <juce_core/juce_core.h>

namespace mm
{
// Preset de fabrica: so os valores que diferem do INIT (o resto volta ao padrao).
// Valores em unidades reais: Hz, segundos, %, indice de escolha, etc.
struct FactoryPreset
{
    const char* name;
    const char* category;
    std::vector<std::pair<juce::String, float>> values;
    juce::StringArray macroNames;   // vazio = AGGRESSION / MOVEMENT / DARKNESS / SPACE
};

const std::vector<FactoryPreset>& getFactoryPresets();
} // namespace mm
