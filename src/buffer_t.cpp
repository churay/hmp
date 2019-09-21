#include <cstring>

#include "buffer_t.h"

namespace llce {

/// Helper Structures ///

struct ring_t { uint64_t regions[2]; };


ring_t calcRingAlloc( const uint64_t pOffset, const uint64_t pLength, const uint64_t pCapacity ) {
    ring_t ring;

    if( pOffset + pLength <= pCapacity ) {
        ring.regions[0] = pLength;
        ring.regions[1] = 0;
    } else {
        ring.regions[0] = pCapacity - pOffset;
        ring.regions[1] = pLength - ring.regions[0];
    }

    return ring;
}


buffer_t::buffer_t( bit8_t* pBuffer, const uint64_t pBufferCapacity ) {
    mBuffer = pBuffer;
    mBufferCapacity = pBufferCapacity;

    mBufferStart = 0;
    mBufferEnd = 0;
}


bool32_t buffer_t::enqueue( const bit8_t* pBuffer, const uint64_t pBufferLength ) {
    bool32_t newBufferFits = pBufferLength <= mBufferCapacity - this->length();

    LLCE_CHECK_WARNING( newBufferFits,
        "Couldn't enqueue additional memory at " << pBuffer << " of size " <<
        pBufferLength << "; insufficient buffer memory remaining." );

    if( newBufferFits ) {
        ring_t bufferAlloc = calcRingAlloc( mBufferEnd, pBufferLength, mBufferCapacity );

        std::memcpy( mBuffer + mBufferEnd, pBuffer + 0, bufferAlloc.regions[0] );
        std::memcpy( mBuffer + 0, pBuffer + bufferAlloc.regions[0], bufferAlloc.regions[1] );

        mBufferEnd = ( mBufferEnd + pBufferLength ) % mBufferCapacity;
    }

    return newBufferFits;
}

bool32_t buffer_t::clear() {
    std::memset( mBuffer, 0, mBufferCapacity );

    mBufferStart = 0;
    mBufferEnd = 0;

    return true;
}


bool32_t buffer_t::dequeue( bit8_t* pBuffer, const uint64_t pBufferLength ) {
    bool32_t hasEnoughData = pBufferLength <= this->length();

    LLCE_CHECK_WARNING( hasEnoughData,
        "Couldn't dequeue requested memory to " << pBuffer << " of size " <<
        pBufferLength << "; insufficient buffer memory available." );

    if( hasEnoughData ) {
        ring_t bufferAlloc = calcRingAlloc( mBufferStart, pBufferLength, mBufferCapacity );

        std::memcpy( pBuffer + 0, mBuffer + mBufferStart, bufferAlloc.regions[0] );
        std::memcpy( pBuffer + bufferAlloc.regions[0], mBuffer + 0, bufferAlloc.regions[1] );

        mBufferStart = ( mBufferStart + pBufferLength ) % mBufferCapacity;
    }

    return hasEnoughData;
}


uint64_t buffer_t::length() const {
    return ( mBufferStart <= mBufferEnd ) ? mBufferEnd - mBufferStart :
        mBufferEnd + mBufferCapacity - mBufferStart;
}


uint64_t buffer_t::capacity() const {
    return mBufferCapacity;
}

};
