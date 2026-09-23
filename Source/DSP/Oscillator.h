#pragma once
#include <juce_core/juce_core.h>
#include <cmath>

namespace mm
{
// =============================================================================
//  Oscillator - oscilador com PolyBLEP (anti-aliasing) para Saw/Square/Pulse.
//  Sem alocacao de memoria: seguro para o audio thread.
//  Preparado para receber Wavetable/FM nas proximas fases (novas formas).
// =============================================================================
class Oscillator
{
public:
    enum class Wave { Sine = 0, Triangle, Saw, Square, Pulse, Noise };

    void prepare (double newSampleRate) noexcept { sampleRate = newSampleRate; }

    void reset (double startPhase = 0.0) noexcept { phase = startPhase; }

    void setFrequency (double hz) noexcept
    {
        increment = juce::jlimit (0.0, 0.5, hz / sampleRate);
    }

    float process (Wave wave) noexcept
    {
        const double t = phase;
        const double dt = increment;
        float out = 0.0f;

        switch (wave)
        {
            case Wave::Sine:
                out = (float) std::sin (juce::MathConstants<double>::twoPi * t);
                break;

            case Wave::Triangle:
                out = (float) (1.0 - 4.0 * std::abs (t - 0.5));
                break;

            case Wave::Saw:
                out = (float) (2.0 * t - 1.0 - polyBlep (t, dt));
                break;

            case Wave::Square:
                out = (float) ((t < 0.5 ? 1.0 : -1.0)
                               + polyBlep (t, dt)
                               - polyBlep (std::fmod (t + 0.5, 1.0), dt));
                break;

            case Wave::Pulse:
            {
                constexpr double width = 0.25;
                double v = (t < width ? 1.0 : -1.0)
                           + polyBlep (t, dt)
                           - polyBlep (std::fmod (t + 1.0 - width, 1.0), dt);
                out = (float) (v - (2.0 * width - 1.0)); // remove DC
                break;
            }

            case Wave::Noise:
                out = random.nextFloat() * 2.0f - 1.0f;
                break;
        }

        phase += dt;
        if (phase >= 1.0)
            phase -= 1.0;

        return out;
    }

private:
    static double polyBlep (double t, double dt) noexcept
    {
        if (dt <= 0.0)
            return 0.0;

        if (t < dt)
        {
            t /= dt;
            return t + t - t * t - 1.0;
        }

        if (t > 1.0 - dt)
        {
            t = (t - 1.0) / dt;
            return t * t + t + t + 1.0;
        }

        return 0.0;
    }

    double sampleRate = 44100.0;
    double phase = 0.0;
    double increment = 0.0;
    juce::Random random;
};
} // namespace mm
