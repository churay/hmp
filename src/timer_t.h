#ifndef LLCE_TIMER_H
#define LLCE_TIMER_H

#include <array>
#include <chrono>
#include <ratio>
#include "consts.h"

namespace llce {

class timer_t {
    public:

    /// Class Attributes ///

    enum class ratio_e : int8_t { fps, spf };
    enum class time_e : int8_t { real, ideal };

    const static uint32_t CACHE_SIZE = 10;

    /// Constructors ///

    timer_t( float64_t pRatio = 60.0, ratio_e pType = ratio_e::fps );

    /// Class Functions ///

    float64_t split();
    float64_t wait( float64_t pTargetFrameTime = -1.0 ) const;

    float64_t ft( time_e pType = time_e::real ) const;
    float64_t tt( time_e pType = time_e::real ) const;
    float64_t fps( time_e pType = time_e::real ) const;

    private:

    /// Class Setup ///

    using Clock = std::chrono::high_resolution_clock;
    using ClockPoint = decltype( Clock::now() );
    using ClockDuration = decltype( Clock::now() - Clock::now() );
    using SecDuration = std::chrono::duration<float64_t, std::ratio<1>>;

    /// Class Fields ///

    ClockDuration mFrameDuration;
    ClockPoint mTimerStart;

    std::array<ClockPoint, CACHE_SIZE> mFrameSplits;
    uint32_t mCurrFrameIdx, mPrevFrameIdx;
};

}

#endif
