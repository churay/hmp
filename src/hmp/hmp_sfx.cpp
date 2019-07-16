#include <cmath>
#include <cstring>

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
    for( uint32_t pWaveIdx = 0; pWaveIdx < MAX_WAVE_COUNT; pWaveIdx++ ) {
        mWaveforms[pWaveIdx] = nullptr;
        mWaveformPositions[pWaveIdx] = 0.0;
        mWaveformDurations[pWaveIdx] = 0.0;
    }

    mRunning = pRunning;
}


void synth_t::update( const float64_t pDT ) {
    for( uint32_t pWaveIdx = 0; pWaveIdx < MAX_WAVE_COUNT; pWaveIdx++ ) {
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


void synth_t::render( const SDL_AudioSpec* pAudioSpec, bit8_t* pAudioBuffer ) const {
    const uint32_t cAudioFormatBytes = SDL_AUDIO_BITSIZE( pAudioSpec->format ) / 8;
    const uint32_t cAudioSampleBytes = cAudioFormatBytes * pAudioSpec->channels;
    const uint32_t cAudioBufferBytes = pAudioSpec->samples * cAudioSampleBytes;

    const bool8_t cIsFormatFloat = SDL_AUDIO_ISFLOAT( pAudioSpec->format );
    const bool8_t cIsFormatSigned = SDL_AUDIO_ISSIGNED( pAudioSpec->format );

    std::memset( pAudioBuffer, 0, cAudioBufferBytes );

    for( uint32_t sampleIdx = 0, bufferIdx = 0; sampleIdx < pAudioSpec->samples; sampleIdx++ ) {
        float64_t sampleDT = ( mUpdateDT * sampleIdx ) / pAudioSpec->samples;

        float64_t sampleValue = 0.0;
        for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT; waveIdx++ ) {
            if( mWaveforms[waveIdx] != nullptr ) {
                float64_t waveTime = mWaveformPositions[waveIdx] + sampleDT;
                if( waveTime <= mWaveformDurations[waveIdx] ) {
                    sampleValue += (*mWaveforms[waveIdx])( waveTime );
                }
            }
        }

        for( uint32_t channelIdx = 0; channelIdx < pAudioSpec->channels; channelIdx++, bufferIdx++ ) {
            bit8_t* sampleAddress = &pAudioBuffer[cAudioFormatBytes * bufferIdx];
            if( cAudioSampleBytes == 1 && cIsFormatSigned ) {
                int8_t* samplePointer = (int8_t*)sampleAddress;
                *samplePointer = static_cast<int8_t>( sampleValue );
            } else if( cAudioSampleBytes == 1 && !cIsFormatSigned ) {
                uint8_t* samplePointer = (uint8_t*)sampleAddress;
                *samplePointer = static_cast<uint8_t>( sampleValue );
            } else if( cAudioSampleBytes == 2 && cIsFormatSigned ) {
                int16_t* samplePointer = (int16_t*)sampleAddress;
                *samplePointer = static_cast<int16_t>( sampleValue );
            } else if( cAudioSampleBytes == 2 && !cIsFormatSigned ) {
                uint16_t* samplePointer = (uint16_t*)sampleAddress;
                *samplePointer = static_cast<uint16_t>( sampleValue );
            } else if( cAudioSampleBytes == 4 && !cIsFormatFloat && !cIsFormatSigned ) {
                int32_t* samplePointer = (int32_t*)sampleAddress;
                *samplePointer = static_cast<int32_t>( sampleValue );
            } else if( cAudioSampleBytes == 4 && !cIsFormatFloat && cIsFormatSigned ) {
                uint32_t* samplePointer = (uint32_t*)sampleAddress;
                *samplePointer = static_cast<uint32_t>( sampleValue );
            } else if( cAudioSampleBytes == 4 && cIsFormatFloat ) {
                float32_t* samplePointer = (float32_t*)sampleAddress;
                *samplePointer = static_cast<float32_t>( sampleValue );
            }
        }
    }
}


void synth_t::play( const waveform_t* pWaveform, const float64_t pWaveDuration ) {
    for( uint32_t pWaveIdx = 0; pWaveIdx < MAX_WAVE_COUNT; pWaveIdx++ ) {
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

float64_t wave::sine( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) {
    return pAmplitude * std::sin( (2 * M_PI * pTime - pPhase) / pWavelength );
}


float64_t wave::square( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) {
    return pAmplitude * ( std::fmod(pTime - pPhase, pWavelength) <= 0.5 ? 1.0 : -1.0 );
}


float64_t wave::triangle( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) {
    return 2.0 * pAmplitude / M_PI * std::asin( std::sin((2 * M_PI * pTime - pPhase) / pWavelength) );
}


float64_t wave::sawtooth( const float64_t pTime, const float64_t pWavelength, const float64_t pAmplitude, const float64_t pPhase ) {
    return 2.0 * pAmplitude / M_PI * std::atan( std::tan((2 * M_PI * pTime - pPhase) / (2.0 * pWavelength)) );
}

};

};
