// =============================================================================
//  PluginTest - host de teste em linha de comando.
//  Carrega o ATDT METAL MACHINE.vst3, toca todos os presets de fabrica e verifica:
//   - o plugin gera audio (nao silencio)   - nenhum NaN/Inf
//   - pico sem clip absurdo               - salvar/restaurar estado funciona
//  Uso: PluginTest "caminho/ATDT METAL MACHINE.vst3"
// =============================================================================
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>
#include <cmath>

using namespace juce;

static void render (AudioPluginInstance& p, int notes[], int numNotes, double seconds, float& peak, float& rms, bool& finite)
{
    const double sr = 48000.0;
    const int block = 512;
    AudioBuffer<float> buffer (2, block);
    const int totalBlocks = (int) (seconds * sr / block);
    double sumSq = 0.0;
    long count = 0;
    peak = 0.0f; finite = true;

    for (int b = 0; b < totalBlocks; ++b)
    {
        MidiBuffer midi;
        if (b == 0)
            for (int i = 0; i < numNotes; ++i) midi.addEvent (MidiMessage::noteOn (1, notes[i], (uint8) 110), 0);
        if (b == totalBlocks * 3 / 4)
            for (int i = 0; i < numNotes; ++i) midi.addEvent (MidiMessage::noteOff (1, notes[i]), 0);

        buffer.clear();
        p.processBlock (buffer, midi);

        for (int c = 0; c < 2; ++c)
        {
            const float* d = buffer.getReadPointer (c);
            for (int i = 0; i < block; ++i)
            {
                if (! std::isfinite (d[i])) finite = false;
                peak = jmax (peak, std::abs (d[i]));
                sumSq += (double) d[i] * d[i];
                ++count;
            }
        }
    }
    rms = (float) std::sqrt (sumSq / (double) jmax (1L, count));
}

int main (int argc, char** argv)
{
    ScopedJuceInitialiser_GUI juce;
    if (argc < 2) { std::printf ("uso: PluginTest <plugin.vst3>\n"); return 2; }

    AudioPluginFormatManager fm;
    fm.addFormat (new VST3PluginFormat());

    OwnedArray<PluginDescription> types;
    VST3PluginFormat vst3;
    vst3.findAllTypesForFile (types, String (argv[1]));
    if (types.isEmpty()) { std::printf ("FALHA: plugin nao encontrado\n"); return 1; }

    String err;
    auto plugin = fm.createPluginInstance (*types[0], 48000.0, 512, err);
    if (plugin == nullptr) { std::printf ("FALHA ao carregar: %s\n", err.toRawUTF8()); return 1; }

    std::printf ("Plugin: %s | instrumento=%d | parametros=%d | programas=%d\n",
                 plugin->getName().toRawUTF8(), (int) types[0]->isInstrument,
                 plugin->getParameters().size(), plugin->getNumPrograms());

    plugin->setPlayConfigDetails (0, 2, 48000.0, 512);
    plugin->prepareToPlay (48000.0, 512);

    int failures = 0;
    int bassNote[] { 40 };          // E2
    int chord[] { 52, 55, 59 };      // Em

    for (int prog = 0; prog < plugin->getNumPrograms(); ++prog)
    {
        plugin->setCurrentProgram (prog);
        {   // espera ~100 ms: o master gain e suavizado (20 ms) para nao gerar clicks
            AudioBuffer<float> settle (2, 512);
            MidiBuffer none;
            for (int i = 0; i < 10; ++i) { settle.clear(); plugin->processBlock (settle, none); }
        }
        const auto name = plugin->getProgramName (prog);
        const bool isBass = name.startsWith ("BASS") || name.startsWith ("DUBSTEP") || name.startsWith ("TRAP");
        float peak, rms; bool finite;
        if (isBass) render (*plugin, bassNote, 1, 2.0, peak, rms, finite);
        else        render (*plugin, chord, 3, 2.0, peak, rms, finite);

        const bool ok = finite && rms > 0.001f && peak < 4.0f;
        if (! ok) ++failures;
        std::printf ("%-4s %-32s peak %6.3f (%+5.1f dBFS)  rms %6.4f %s\n", ok ? "OK" : "FAIL", name.toRawUTF8(),
                     peak, Decibels::gainToDecibels (peak, -100.0f), rms, finite ? "" : "NaN/Inf!");

        // silencio apos o release (voz nao fica presa)
        AudioBuffer<float> tail (2, 512);
        MidiBuffer none;
        float tailPeak = 0.0f;
        for (int i = 0; i < 400; ++i) { tail.clear(); plugin->processBlock (tail, none); }
        for (int c = 0; c < 2; ++c) tailPeak = jmax (tailPeak, tail.getMagnitude (c, 0, 512));
        if (tailPeak > 0.001f) { ++failures; std::printf ("     FAIL: nota presa (tail %.4f)\n", tailPeak); }
    }

    // estado: salvar, mudar, restaurar
    plugin->setCurrentProgram (3);
    MemoryBlock state;
    plugin->getStateInformation (state);
    const float before = plugin->getParameters()[10]->getValue();
    plugin->setCurrentProgram (0);
    plugin->setStateInformation (state.getData(), (int) state.getSize());
    const float after = plugin->getParameters()[10]->getValue();
    const bool stateOk = std::abs (before - after) < 1.0e-5f && state.getSize() > 100;
    if (! stateOk) ++failures;
    std::printf ("%-4s Estado salvo/restaurado (%d bytes)\n", stateOk ? "OK" : "FAIL", (int) state.getSize());

    plugin->releaseResources();
    std::printf ("\nRESULTADO: %s (%d falha(s))\n", failures == 0 ? "TUDO OK" : "FALHOU", failures);
    return failures == 0 ? 0 : 1;
}
