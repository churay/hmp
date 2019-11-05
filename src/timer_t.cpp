#include <cmath>
#include <chrono>
#include <ratio>
#include <thread>

#include "timer_t.h"

namespace llce {

timer_t::timer_t( float64_t pRatio, timer_t::ratio_e pType ) {
    LLCE_CHECK_ERROR( pRatio > 0.0,
        "Couldn't create timer with invalid fps/spf ratio of " << pRatio << "; "
        "this ratio value must be positive." );

    SecDuration frameDuration( (pType == timer_t::ratio_e::spf) ? pRatio : 1.0 / pRatio );
    mFrameDuration = std::chrono::duration_cast<ClockDuration>( frameDuration );

    ClockPoint initTime = Clock::now();
    mTimerStart = initTime;
    mFrameSplits.fill( initTime );
    mCurrFrameIdx = mPrevFrameIdx = 0;
}


float64_t timer_t::split() {
    mPrevFrameIdx = mCurrFrameIdx;
    mCurrFrameIdx = ( mCurrFrameIdx + 1 ) % mFrameSplits.size();
    mFrameSplits[mCurrFrameIdx] = Clock::now();

    SecDuration splitSecs = std::chrono::duration_cast<SecDuration>(
        mFrameSplits[mCurrFrameIdx] - mFrameSplits[mPrevFrameIdx] );
    return static_cast<float64_t>( splitSecs.count() );
}


float64_t timer_t::wait( float64_t pTargetFrameTime ) const {
    ClockDuration waitTime = ( pTargetFrameTime >= 0.0 ) ?
        std::chrono::duration_cast<ClockDuration>( SecDuration{pTargetFrameTime} ) :
        mFrameDuration - (mFrameSplits[mCurrFrameIdx] - mFrameSplits[mPrevFrameIdx]);

    std::this_thread::sleep_for( waitTime );

    SecDuration waitSecs = std::chrono::duration_cast<SecDuration>( waitTime );
    return static_cast<float64_t>( waitSecs.count() );
}


float64_t timer_t::ft( timer_t::time_e pType ) const {
    ClockDuration prevFrameTime = ( pType == timer_t::time_e::ideal ) ? mFrameDuration :
        std::max( mFrameDuration, mFrameSplits[mCurrFrameIdx] - mFrameSplits[mPrevFrameIdx] );
    SecDuration prevFrameSecs = std::chrono::duration_cast<SecDuration>( prevFrameTime );
    return static_cast<float64_t>( prevFrameSecs.count() );
}


float64_t timer_t::tt( timer_t::time_e pType ) const {
    ClockDuration totalTime = mFrameSplits[mCurrFrameIdx] - mTimerStart;
    SecDuration totalSecs = std::chrono::duration_cast<SecDuration>(
        (pType == timer_t::time_e::real) ? totalTime :
        std::ceil( totalTime.count() / mFrameDuration.count() ) * mFrameDuration );
    return static_cast<float64_t>( totalSecs.count() );
}


float64_t timer_t::fps( timer_t::time_e pType ) const {
    return 1.0 / ft( pType );
}

}
