#include "platform.h"

#include "memory_t.h"

namespace llce {

memory_t::memory_t( uint64_t pBlockCount, const uint64_t* pBlockLengths, bit8_t* pBlockBase ) {
    LLCE_ASSERT_DEBUG( pBlockCount <= memory_t::MAX_BLOCKS,
        "Couldn't allocate memory chunk with " << pBlockCount << " blocks; " <<
        "block count has a maximum value of " << memory_t::MAX_BLOCKS << "." );

    mBlockCount = pBlockCount;
    mBufferLength = 0;
    for( uint64_t blockIdx = 0; blockIdx < pBlockCount; blockIdx++ ) {
        mBlockLengths[blockIdx] = pBlockLengths[blockIdx];
        mBlockAllocs[blockIdx] = 0;
        mBufferLength += pBlockLengths[blockIdx];
    }

    mBuffer = platform::allocBuffer( mBufferLength, pBlockBase );
    LLCE_ASSERT_DEBUG( mBuffer != nullptr,
        "Unable to allocate buffer of length " << mBufferLength << " " <<
        "at base address " << pBlockBase << ".");

    mBlockBuffers[0] = mBuffer;
    for( uint64_t blockIdx = 1; blockIdx < pBlockCount; blockIdx++ ) {
        mBlockBuffers[blockIdx] = mBlockBuffers[blockIdx-1] + mBlockLengths[blockIdx-1];
    }
}


memory_t::~memory_t() {
    platform::deallocBuffer( mBuffer, mBufferLength );
}


bit8_t* memory_t::allocate( uint64_t pBufferIdx, uint64_t pAllocLength ) {
    LLCE_ASSERT_DEBUG(
        mBlockAllocs[pBufferIdx] + pAllocLength <= mBlockLengths[pBufferIdx],
        "Cannot allocate an additional " << pAllocLength << " bytes to buffer " <<
        pBufferIdx << "; allocation puts block over " << mBlockAllocs[pBufferIdx] <<
        "/" << mBlockLengths[pBufferIdx] << " allocation capacity." );

    bit8_t* allocAddress = mBlockBuffers[pBufferIdx] + mBlockAllocs[pBufferIdx];
    mBlockAllocs[pBufferIdx] += pAllocLength;
    return allocAddress;
}


bit8_t* memory_t::buffer( uint64_t pBufferIdx ) const {
    return ( pBufferIdx < mBlockCount ) ? mBlockBuffers[pBufferIdx] : mBuffer;
}


uint64_t memory_t::length( uint64_t pBufferIdx ) const {
    return ( pBufferIdx < mBlockCount ) ? mBlockLengths[pBufferIdx] : mBufferLength;
}

}
