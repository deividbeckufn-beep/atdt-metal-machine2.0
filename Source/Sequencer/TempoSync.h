#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

namespace mm
{
// =============================================================================
//  TempoSync - le BPM, transporte e posicao (PPQ) do REAPER a cada bloco.
//  Fase 1: apenas leitura/exibicao. Fase 3: base do Step Sequencer.
// =============================================================================
class TempoSync
{
public:
    void update (juce::AudioPlayHead* playHead) noexcept
    {
        if (playHead == nullptr)
        {
            hostHasTempo.store (false);
            return;
        }

        if (auto pos = playHead->getPosition())
        {
            if (auto b = pos->getBpm())
            {
                bpm.store (*b);
                hostHasTempo.store (true);
            }
            if (auto p = pos->getPpqPosition())
                ppq.store (*p);

            playing.store (pos->getIsPlaying());
        }
    }

    double getBpm() const noexcept         { return bpm.load(); }
    double getPpq() const noexcept         { return ppq.load(); }
    bool   isPlaying() const noexcept      { return playing.load(); }
    bool   hostProvidesTempo() const noexcept { return hostHasTempo.load(); }

private:
    std::atomic<double> bpm { 120.0 };
    std::atomic<double> ppq { 0.0 };
    std::atomic<bool>   playing { false };
    std::atomic<bool>   hostHasTempo { false };
};
} // namespace mm
