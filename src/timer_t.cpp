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


void timer_t::split() {
    mPrevFrameIdx = mCurrFrameIdx;
    mCurrFrameIdx = ( mCurrFrameIdx + 1 ) % mFrameSplits.size();
    mFrameSplits[mCurrFrameIdx] = Clock::now();
}


void timer_t::wait( float64_t pTargetFrameTime ) const {
    ClockDuration frameTime = ( pTargetFrameTime < 0.0 ) ?
        ClockDuration( static_cast<int64_t>(pTargetFrameTime) ) :
        mFrameSplits[mCurrFrameIdx] - mFrameSplits[mPrevFrameIdx];
    ClockDuration remainingTime = mFrameDuration - frameTime;
    std::this_thread::sleep_for( remainingTime );
}


float64_t timer_t::ft( timer_t::time_e pType ) const {
    ClockDuration prevFrameTime = ( pType == timer_t::time_e::ideal ) ? mFrameDuration :
        std::max( mFrameDuration, mFrameSplits[mCurrFrameIdx] - mFrameSplits[mPrevFrameIdx] );
    SecDuration prevFrameSecs = std::chrono::duration_cast<SecDuration>( prevFrameTime );
    return static_cast<float64_t>( prevFrameSecs.count() );
}


float64_t timer_t::tt( timer_t::time_e pType ) const {
    // TODO(JRC): For true idealized time, round this up to the nearest frame time.
    SecDuration totalTime = std::chrono::duration_cast<SecDuration>(
        ((pType == timer_t::time_e::ideal) ? mFrameSplits[mCurrFrameIdx] : Clock::now() ) -
        mTimerStart );
    return static_cast<float64_t>( totalTime.count() );
}


float64_t timer_t::fps( timer_t::time_e pType ) const {
    return 1.0 / ft( pType );
}

}
