#include "hmp_sfx.h"

namespace hmp {

namespace sfx {

/// 'hmp::sfx::waveform_t' Functions ///

waveform_t::waveform_t( const wave_f pWaveFun, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) :
        mWaveFun( pWaveFun ), mWavelength( pWavelength ), mAmplitude( pAmplitude ), mPhase( pPhase ) {
    
}


float64_t waveform_t::operator()( const float64_t pTime ) const {
    return mWaveFun( pTime, mWavelength, mAmplitude, mPhase );
}

/// 'hmp::sfx::synth_t' Functions ///

synth_t::synth_t( const bool32_t pRunning ) {
    for( uint32_t pWaveIdx = 0; pWaveIdx < MAX_CHANNEL_COUNT; pWaveIdx++ ) {
        mWaveforms[pWaveIdx] = nullptr;
        mWaveformPositions[pWaveIdx] = 0.0;
        mWaveformDurations[pWaveIdx] = 0.0;
    }

    mRunning = pRunning;
}


void synth_t::update( const float64_t pDT ) {
    for( uint32_t pWaveIdx = 0; pWaveIdx < MAX_CHANNEL_COUNT; pWaveIdx++ ) {
        if( mWaveformPositions[pWaveIdx] > mWaveformDurations[pWaveIdx] ) {
            mWaveforms[pWaveIdx] = nullptr;
            mWaveformPositions[pWaveIdx] = 0.0;
            mWaveformDurations[pWaveIdx] = 0.0;
        } else {
            mWaveformPositions[pWaveIdx] += pDT;
        }
    }

    mUpdateDT = pDT;
}


void synth_t::render() {
    // TODO(JRC)
}


void synth_t::play( const waveform_t* pWaveform, const float64_t pWaveDuration ) {
    for( uint32_t pWaveIdx = 0; pWaveIdx < MAX_CHANNEL_COUNT; pWaveIdx++ ) {
        if( mWaveforms[pWaveIdx] == nullptr ) {
            mWaveforms[pWaveIdx] = pWaveform;
            mWaveformPositions[pWaveIdx] = 0.0;
            mWaveformDurations[pWaveIdx] = pWaveDuration;
        }
    }
}


void synth_t::toggle() {
    mRunning = !mRunning;
}

/// 'hmp::sfx::wave' Functions ///

float64_t wave::square( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) {
    return 0.0;
}


float64_t wave::sine( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) {
    return 0.0;
}


float64_t wave::triangle( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) {
    return 0.0;
}


float64_t wave::sawtooth( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) {
    return 0.0;
}

};

};
