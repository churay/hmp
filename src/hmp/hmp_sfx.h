#ifndef HMP_SFX_H
#define HMP_SFX_H

#include <SDL2/SDL.h>

#include "hmp_consts.h"

namespace hmp {

namespace sfx {

typedef float64_t (*wave_f)( const float64_t, const float64_t, const float64_t, const float64_t );

struct waveform_t {
    waveform_t( const wave_f pWaveFun, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase );

    float64_t operator()( const float64_t pTime ) const;

    wave_f mWaveFun;
    float64_t mWavelength, mAmplitude, mPhase;
};

struct synth_t {
    constexpr static uint32_t MAX_WAVE_COUNT = 4;

    synth_t( const bool32_t pRunning = true );

    bool32_t update( const float64_t pDT );
    bool32_t render( const SDL_AudioSpec& pAudioSpec, bit8_t* pAudioBuffer ) const;

    bool32_t playing() const;

    void play( const waveform_t* pWaveform, const float64_t pWaveDuration );
    void toggle();

    const waveform_t* mWaveforms[MAX_WAVE_COUNT];
    float64_t mWaveformPositions[MAX_WAVE_COUNT];
    float64_t mWaveformDurations[MAX_WAVE_COUNT];

    bool32_t mRunning;
    float64_t mUpdateDT;
};

namespace wave {
    float64_t sine( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase );
    float64_t square( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase );
    float64_t triangle( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase );
    float64_t sawtooth( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase );
};

};

};

#endif