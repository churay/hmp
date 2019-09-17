#include <cmath>
#include <cstring>

#include "hmp_sfx.h"

namespace hmp {

namespace sfx {

/// 'hmp::sfx::waveform_t' Functions ///

waveform_t::waveform_t( const wave_f pWaveFun, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase ) :
        mWaveFun( pWaveFun ), mWavelength( pFrequency ), mAmplitude( pAmplitude ), mPhase( pPhase ) {
    
}


float64_t waveform_t::operator()( const float64_t pTime ) const {
    return mWaveFun( pTime, mWavelength, mAmplitude, mPhase );
}

/// 'hmp::sfx::synth_t' Functions ///

synth_t::synth_t( const bool32_t pRunning ) {
    for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT; waveIdx++ ) {
        mWaveforms[waveIdx] = nullptr;
        mWaveformPositions[waveIdx] = 0.0;
        mWaveformDurations[waveIdx] = 0.0;
    }

    mRunning = pRunning;
}


bool32_t synth_t::update( const float64_t pDT ) {
    for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT; waveIdx++ ) {
        if( mWaveformPositions[waveIdx] > mWaveformDurations[waveIdx] ) {
            mWaveforms[waveIdx] = nullptr;
            mWaveformPositions[waveIdx] = 0.0;
            mWaveformDurations[waveIdx] = 0.0;
        } else if( mWaveforms[waveIdx] != nullptr ) {
            mWaveformPositions[waveIdx] += pDT;
        }
    }

    mUpdateDT = pDT;

    return true;
}


bool32_t synth_t::render( const SDL_AudioSpec& pAudioSpec, bit8_t* pAudioBuffer ) const {
    const uint32_t cAudioFormatBytes = SDL_AUDIO_BITSIZE( pAudioSpec.format ) / 8;
    const uint32_t cAudioSampleBytes = cAudioFormatBytes * pAudioSpec.channels;
    const uint32_t cAudioRenderSamples = std::ceil( mUpdateDT * pAudioSpec.freq );
    const uint32_t cAudioBufferBytes = cAudioRenderSamples * cAudioSampleBytes;

    const bool8_t cIsFormatFloat = SDL_AUDIO_ISFLOAT( pAudioSpec.format );
    const bool8_t cIsFormatSigned = SDL_AUDIO_ISSIGNED( pAudioSpec.format );

    std::memset( pAudioBuffer, 0, cAudioBufferBytes );

    for( uint32_t sampleIdx = 0, bufferIdx = 0; sampleIdx < cAudioRenderSamples; sampleIdx++ ) {
        float64_t sampleDT = ( mUpdateDT * sampleIdx ) / cAudioRenderSamples;

        float64_t sampleValue = 0.0;
        for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT; waveIdx++ ) {
            if( mWaveforms[waveIdx] != nullptr ) {
                float64_t waveTime = mWaveformPositions[waveIdx] - mUpdateDT + sampleDT;
                if( waveTime <= mWaveformDurations[waveIdx] ) {
                    sampleValue += (*mWaveforms[waveIdx])( waveTime );
                }
            }
        }

        for( uint32_t channelIdx = 0; channelIdx < pAudioSpec.channels; channelIdx++, bufferIdx++ ) {
            bit8_t* sampleAddress = &pAudioBuffer[cAudioFormatBytes * bufferIdx];
            if( cAudioFormatBytes == 1 && cIsFormatSigned ) {
                int8_t* samplePointer = (int8_t*)sampleAddress;
                *samplePointer = static_cast<int8_t>( sampleValue );
            } else if( cAudioFormatBytes == 1 && !cIsFormatSigned ) {
                uint8_t* samplePointer = (uint8_t*)sampleAddress;
                *samplePointer = static_cast<uint8_t>( sampleValue );
            } else if( cAudioFormatBytes == 2 && cIsFormatSigned ) {
                int16_t* samplePointer = (int16_t*)sampleAddress;
                *samplePointer = static_cast<int16_t>( sampleValue );
            } else if( cAudioFormatBytes == 2 && !cIsFormatSigned ) {
                uint16_t* samplePointer = (uint16_t*)sampleAddress;
                *samplePointer = static_cast<uint16_t>( sampleValue );
            } else if( cAudioFormatBytes == 4 && !cIsFormatFloat && !cIsFormatSigned ) {
                int32_t* samplePointer = (int32_t*)sampleAddress;
                *samplePointer = static_cast<int32_t>( sampleValue );
            } else if( cAudioFormatBytes == 4 && !cIsFormatFloat && cIsFormatSigned ) {
                uint32_t* samplePointer = (uint32_t*)sampleAddress;
                *samplePointer = static_cast<uint32_t>( sampleValue );
            } else if( cAudioFormatBytes == 4 && cIsFormatFloat ) {
                float32_t* samplePointer = (float32_t*)sampleAddress;
                *samplePointer = static_cast<float32_t>( sampleValue );
            }
        }
    }

    return true;
}

bool32_t synth_t::playing() const {
    bool32_t isPlaying = false;
    for( uint32_t waveIdx = 0, wavePlaying = 0; waveIdx < MAX_WAVE_COUNT; waveIdx++ ) {
        isPlaying |= mWaveforms[waveIdx] != nullptr;
    }
    return isPlaying && mRunning;
}


void synth_t::play( const waveform_t* pWaveform, const float64_t pWaveDuration ) {
    bool wavePlaying = false;
    for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT && !wavePlaying; waveIdx++ ) {
        wavePlaying |= mWaveforms[waveIdx] == pWaveform;
    } for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT && !wavePlaying; waveIdx++ ) {
        if( mWaveforms[waveIdx] == nullptr ) {
            mWaveforms[waveIdx] = pWaveform;
            mWaveformPositions[waveIdx] = 0.0;
            mWaveformDurations[waveIdx] = pWaveDuration;
            wavePlaying = true;
        }
    }
}


void synth_t::toggle() {
    mRunning = !mRunning;
}

/// 'hmp::sfx::wave' Functions ///

float64_t wave::sine( const float64_t pTime, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase ) {
    return pAmplitude * std::sin( 2 * M_PI * pFrequency * pTime - pPhase );
}


float64_t wave::square( const float64_t pTime, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase ) {
    const float64_t pPeriod = 1.0 / pFrequency;
    return pAmplitude * ( std::fmod(pTime - pPhase, pPeriod) <= (pPeriod / 2.0) ? 1.0 : -1.0 );
}


float64_t wave::triangle( const float64_t pTime, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase ) {
    return pAmplitude / M_PI_2 * std::asin( std::sin(2 * M_PI * pFrequency * pTime - pPhase) );
}


float64_t wave::sawtooth( const float64_t pTime, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase ) {
    return pAmplitude / M_PI_2 * std::atan( std::tan(M_PI * pFrequency * pTime - pPhase) );
}

};

};
