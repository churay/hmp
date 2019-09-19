#include "buffer_t.h"

namespace llce {

buffer_t::buffer_t( bit8_t* pBuffer, const uint64_t pBufferCapacity ) {
    mBuffer = pBuffer;
    mBufferCapacity = pBufferCapacity;

    mBufferStart = 0;
    mBufferEnd = 0;
}


bool32_t buffer_t::enqueue( const bit8_t* pBuffer, const uint64_t pBufferLength ) {
    // TODO(JRC)
    return false;
}


bool32_t buffer_t::dequeue( bit8_t* pBuffer, const uint64_t pBufferLength ) {
    // TODO(JRC)
    return false;
}


uint64_t buffer_t::length() const {
    return ( mBufferStart <= mBufferEnd ) ? mBufferEnd - mBufferStart :
        mBufferEnd + mBufferCapacity - mBufferStart;
}

};
