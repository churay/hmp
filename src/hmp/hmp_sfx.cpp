#include "hmp_sfx.h"

namespace hmp {

namespace sfx {

/// 'hmp::sfx::synth_t' Functions ///

synth_t::synth_t( const bool32_t pRunning ) {
    for( uint32_t pWaveIdx = 0; pWaveIdx < MAX_CHANNEL_COUNT; pWaveIdx++ ) {
        mWaveforms[pWaveIdx] = nullptr;
        mWaveformPositions[pWaveIdx] = 0.0f;
        mWaveformDurations[pWaveIdx] = 0.0f;
    }

    mRunning = pRunning;
}


void synth_t::update( const float64_t pDT ) {
    // TODO(JRC)
}


void synth_t::render() {
    // TODO(JRC)
}


void synth_t::play( const wave_f pWaveform, const float64_t pWaveDuration ) {
    // TODO(JRC)
}


void synth_t::toggle() {
    mRunning = !mRunning;
}

};

};
