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

    mTimerStart = Clock::now();
    mFrameSplits.push_back( mTimerStart );
}


float64_t timer_t::split() {
    mFrameSplits.push_back( Clock::now() );
    SecDuration splitSecs = std::chrono::duration_cast<SecDuration>(
        mFrameSplits.back(0) - mFrameSplits.back(1) );
    return static_cast<float64_t>( splitSecs.count() );
}


float64_t timer_t::wait( float64_t pRatio, timer_t::ratio_e pType ) const {
    SecDuration frameSecs( (pType == timer_t::ratio_e::spf) ? pRatio : 1.0 / pRatio );
    ClockDuration frameDuration = ( pRatio == 0.0 ) ? mFrameDuration :
        std::chrono::duration_cast<ClockDuration>( frameSecs );

    ClockDuration waitTime = frameDuration - ( mFrameSplits.back(0) - mFrameSplits.back(1) );
    std::this_thread::sleep_for( waitTime );

    SecDuration waitSecs = std::chrono::duration_cast<SecDuration>( waitTime );
    return static_cast<float64_t>( waitSecs.count() );
}


float64_t timer_t::ft( timer_t::time_e pType ) const {
    ClockDuration prevFrameTime = ( pType == timer_t::time_e::ideal ) ? mFrameDuration :
        std::max( mFrameDuration, mFrameSplits.back(0) - mFrameSplits.back(1) );
    SecDuration prevFrameSecs = std::chrono::duration_cast<SecDuration>( prevFrameTime );
    return static_cast<float64_t>( prevFrameSecs.count() );
}


float64_t timer_t::tt( timer_t::time_e pType ) const {
    ClockDuration totalTime = mFrameSplits.back( 0 ) - mTimerStart;
    SecDuration totalSecs = std::chrono::duration_cast<SecDuration>(
        (pType == timer_t::time_e::real) ? totalTime :
        std::ceil( totalTime.count() / mFrameDuration.count() ) * mFrameDuration );
    return static_cast<float64_t>( totalSecs.count() );
}


float64_t timer_t::fps( timer_t::time_e pType ) const {
    return 1.0 / ft( pType );
}

}
