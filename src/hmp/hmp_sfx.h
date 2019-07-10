#ifndef HMP_SFX_H
#define HMP_SFX_H

#include "hmp_consts.h"

namespace hmp {

namespace sfx {

constexpr static uint32_t MAX_CHANNEL_COUNT = 4;

typedef float64_t (*wave_f)( float64_t );

struct synth_t {
    synth_t( const bool32_t pRunning = true );

    void update( const float64_t pDT );
    void render();

    void play( const wave_f pWaveform, const float64_t pWaveDuration );
    void toggle();

    wave_f mWaveforms[MAX_CHANNEL_COUNT];
    float64_t mWaveformPositions[MAX_CHANNEL_COUNT];
    float64_t mWaveformDurations[MAX_CHANNEL_COUNT];
    bool32_t mRunning;
};

};

};

#endif
