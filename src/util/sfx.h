#ifndef LLCE_SFX_H
#define LLCE_SFX_H

#include <SDL2/SDL.h>

#include "consts.h"

namespace llce {

namespace sfx {

/// Namespace Macros ///

// NOTE(JRC): In order to ensure that pure 'wave' functions (i.e. float64_t -> float64_t)
// can be generated from 'waveform' functions (i.e. float64_t x 4 -> float64_t),
// a template function is required for each such 'waveform' function. These internals
// don't change between templates, so a preprocessor macro packages this implementation
// for reuse.
#define LLCE_SFX_WAVEFORM_WAVE(waveform_name) \
    template <char8_t Note, int8_t Sign = 0, uint8_t Octave = 4> \
    float64_t waveform_name( const float64_t pTime ) { \
        const static float64_t csFrequency = llce::sfx::freq( Note, Sign, Octave ); \
        return llce::sfx::waveform::waveform_name( pTime, csFrequency, 1.0, 0.0 ); \
    }

/// Namespace Attributes ///

typedef float64_t (*wave_f)( const float64_t );

/// Namespace Types ///

struct synth_t {
    constexpr static uint32_t MAX_WAVE_COUNT = 4;

    synth_t( const float32_t* pVolume = nullptr, const bool32_t pRunning = true );

    bool32_t update( const float64_t pDT, const uint32_t pDF );
    bool32_t render( const SDL_AudioSpec& pAudioSpec, bit8_t* pAudioBuffer ) const;

    bool32_t playing() const;

    void play( const wave_f pWave, const float64_t pWaveDuration );
    void toggle();

    wave_f mWaves[MAX_WAVE_COUNT];
    float64_t mWavePositions[MAX_WAVE_COUNT];
    float64_t mWaveDurations[MAX_WAVE_COUNT];

    const float32_t* mVolume;
    bool32_t mRunning;

    float64_t mUpdateDT;
    uint32_t mUpdateDF;
    int32_t mLatency;
};

/// Namespace Functions ///

float64_t freq( const char8_t pNote, const int8_t pSign = 0, const uint8_t pOctave = 4 );

namespace waveform {
    float64_t sine( const float64_t pTime, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase );
    float64_t square( const float64_t pTime, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase );
    float64_t triangle( const float64_t pTime, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase );
    float64_t sawtooth( const float64_t pTime, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase );

    // TODO(JRC): It would be nice if this process could be automated (e.g. for
    // each function in the 'waveform' namespace), but it seems hard in C++
    // without using a very clumsy and/or overengineering solution.
    LLCE_SFX_WAVEFORM_WAVE( sine );
    LLCE_SFX_WAVEFORM_WAVE( square );
    LLCE_SFX_WAVEFORM_WAVE( triangle );
    LLCE_SFX_WAVEFORM_WAVE( sawtooth );
};

};

};

#endif
