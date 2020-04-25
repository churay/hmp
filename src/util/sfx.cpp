#include <cmath>
#include <cstring>

#include "sfx.h"

namespace llce {

namespace sfx {

/// Namespace Attributes ///

/// 'llce::sfx::waveform_t' Functions ///

waveform_t::waveform_t( const wave_f pWaveFun, const float64_t pFrequency, const float64_t pAmplitude, const float64_t pPhase ) :
        mWaveFun( pWaveFun ), mWavelength( pFrequency ), mAmplitude( pAmplitude ), mPhase( pPhase ) {
    
}


float64_t waveform_t::operator()( const float64_t pTime ) const {
    return mWaveFun( pTime, mWavelength, mAmplitude, mPhase );
}

/// 'llce::sfx::synth_t' Functions ///

synth_t::synth_t( const bool32_t pRunning ) {
    for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT; waveIdx++ ) {
        mWaveforms[waveIdx] = nullptr;
        mWaveformPositions[waveIdx] = 0.0;
        mWaveformDurations[waveIdx] = 0.0;
    }

    mRunning = pRunning;
    mUpdateDT = 0.0;
    mUpdateDF = 0;
    mLatency = 0;
}


bool32_t synth_t::update( const float64_t pDT, const uint32_t pDF ) {
    for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT; waveIdx++ ) {
        if( mWaveforms[waveIdx] != nullptr ) {
            if( mWaveformPositions[waveIdx] > mWaveformDurations[waveIdx] ) {
                mWaveforms[waveIdx] = nullptr;
                mWaveformPositions[waveIdx] = 0.0;
                mWaveformDurations[waveIdx] = 0.0;
            } else {
                mWaveformPositions[waveIdx] += mUpdateDT * mUpdateDF;
            }
        }
    }

    mLatency += pDF - 1;
    mUpdateDT = pDT;
    mUpdateDF = pDF;

    return true;
}


bool32_t synth_t::render( const SDL_AudioSpec& pAudioSpec, bit8_t* pAudioBuffer ) const {
    const float64_t cAudioDT = mUpdateDF * mUpdateDT;
    const uint32_t cAudioFormatBytes = SDL_AUDIO_BITSIZE( pAudioSpec.format ) / 8;
    const uint32_t cAudioSampleBytes = cAudioFormatBytes * pAudioSpec.channels;
    const uint32_t cAudioRenderSamples = std::ceil( cAudioDT * pAudioSpec.freq );
    const uint32_t cAudioBufferBytes = cAudioRenderSamples * cAudioSampleBytes;

    const bool8_t cIsFormatFloat = SDL_AUDIO_ISFLOAT( pAudioSpec.format );
    const bool8_t cIsFormatSigned = SDL_AUDIO_ISSIGNED( pAudioSpec.format );

    std::memset( pAudioBuffer, 0, cAudioBufferBytes );

    for( uint32_t sampleIdx = 0, bufferIdx = 0; sampleIdx < cAudioRenderSamples; sampleIdx++ ) {
        float64_t sampleDT = ( cAudioDT * sampleIdx ) / cAudioRenderSamples;

        float64_t sampleValue = 0.0;
        for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT; waveIdx++ ) {
            if( mWaveforms[waveIdx] != nullptr ) {
                float64_t waveTime = mWaveformPositions[waveIdx] + sampleDT;
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
    uint32_t waveformIdx = MAX_WAVE_COUNT;

    for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT && waveformIdx >= MAX_WAVE_COUNT; waveIdx++ ) {
        waveformIdx = ( mWaveforms[waveIdx] == pWaveform ) ? waveIdx : waveformIdx;
    } for( uint32_t waveIdx = 0; waveIdx < MAX_WAVE_COUNT && waveformIdx >= MAX_WAVE_COUNT; waveIdx++ ) {
        if( mWaveforms[waveIdx] == nullptr ) {
            mWaveforms[waveIdx] = pWaveform;
            waveformIdx = waveIdx;
        }
    }

    // NOTE(JRC): This code resets playing sounds if they self-interrupt.
    if( waveformIdx < MAX_WAVE_COUNT ) {
        mWaveformPositions[waveformIdx] = 0.0;
        mWaveformDurations[waveformIdx] = pWaveDuration;
    }
}


void synth_t::toggle() {
    // TODO(JRC): This behavior has been broken by the lookahead buffer implementation.
    mRunning = !mRunning;
}

/// 'llce::sfx' Functions ///

float64_t freq( const char8_t pNote, const int8_t pSign, const uint8_t pOctave ) {
    // NOTE(JRC): Standard piano keys only span 9 octaves (i.e. [0, 8]), but we
    // allow a few more to span the human auditory range (i.e. <= 20k Hz).
    const static uint8_t csMaxOctave = 10;
    const static uint8_t csOctaveMajorCount = 7, csOctaveMinorCount = 5;
    const static uint8_t csOctaveNoteCount = csOctaveMajorCount + csOctaveMinorCount;

    LLCE_CHECK_WARNING( pOctave <= csMaxOctave,
        "The provided octave level '" << pOctave << "' is outside " <<
        "the supported range [0, " << csMaxOctave << "]." );
    LLCE_CHECK_WARNING( 'a' <= pNote && pNote <= 'g',
        "The provided note identifier '" << pNote << "' is outside " <<
        "the standard note range ['a', 'g']" );

    // NOTE(JRC): 'noteOctaveIndex' is the index of the lettered note within the
    // octave, which needs to be adjusted based on the weird standard octave range
    // `['c', 'g'] U ['a', 'b']` and the minor gap at 'e' (which has no sharp).
    uint8_t noteOctaveIndex = 0; {
        noteOctaveIndex = ( pNote < 'c' ) ?
            ( 'g' - 'c' ) + ( pNote - 'a' ) + 1 : pNote - 'c';
        noteOctaveIndex = 2 * noteOctaveIndex - (
            (noteOctaveIndex > ('e' - 'c')) ? 1 : 0 );
    }
    const uint8_t cNoteIndex = glm::clamp(
        1 + csOctaveNoteCount * pOctave + noteOctaveIndex + pSign,
        1, (csMaxOctave + 1) * csOctaveNoteCount );

    // This formula was derived from the frequency formula for standard piano keys:
    // https://en.wikipedia.org/wiki/Piano_key_frequencies
    const static float64_t csBaseFrequency = 440.0;
    const static float64_t csStepFrequency = std::pow( 2.0, 1.0 / 12.0 );
    return csBaseFrequency * std::pow( csStepFrequency, cNoteIndex - 58.0 );
}

/// 'llce::sfx::wave' Functions ///

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
