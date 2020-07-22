#include "platform.h"

#include "memory_t.h"

namespace llce {

memory_t::memory_t( uint64_t pBufferLength, bit8_t* pBufferBase ) :
        memory_t( &pBufferLength, 1, pBufferBase ) {
    
}


memory_t::memory_t( const uint64_t* pPartitionLengths, uint64_t pPartitionCount, bit8_t* pBufferBase ) {
    LLCE_CHECK_ERROR( pPartitionCount <= memory_t::MAX_PARTITIONS,
        "Couldn't allocate memory chunk with " << pPartitionCount << " partitions; " <<
        "patition count has a maximum value of " << memory_t::MAX_PARTITIONS << "." );

    // NOTE(JRC): This is a "belt and suspenders" safety precaution that fills unused
    // buffers with zero/null values to help identify illegal partition access.
    for( uint64_t partitionIdx = 0; partitionIdx < MAX_PARTITIONS; partitionIdx++ ) {
        mPartitionBuffers[partitionIdx] = nullptr;
        mPartitionLengths[partitionIdx] = 0;
        mPartitionStacks[partitionIdx] = nullptr;
        mPartitionHeaps[partitionIdx] = nullptr;
    }

    mPartitionCount = pPartitionCount;
    mBufferLength = 0;
    for( uint64_t partitionIdx = 0; partitionIdx < mPartitionCount; partitionIdx++ ) {
        mPartitionLengths[partitionIdx] = pPartitionLengths[partitionIdx];
        mBufferLength += pPartitionLengths[partitionIdx];
    }

    mBuffer = platform::allocBuffer( mBufferLength, pBufferBase );
    LLCE_CHECK_ERROR( mBuffer != nullptr,
        "Unable to allocate buffer of length " << mBufferLength << " " <<
        "at base address " << pBufferBase << ".");

    for( uint64_t partitionIdx = 0; partitionIdx < mPartitionCount; partitionIdx++ ) {
        mPartitionBuffers[partitionIdx] = mPartitionHeaps[partitionIdx] =
            ( partitionIdx != 0 ) ? mPartitionStacks[partitionIdx - 1] : mBuffer;
        mPartitionStacks[partitionIdx] =
            mPartitionBuffers[partitionIdx] + mPartitionLengths[partitionIdx];
    }
}


memory_t::~memory_t() {
    platform::deallocBuffer( mBuffer, mBufferLength );
}


bit8_t* memory_t::salloc( uint64_t pAllocLength, uint64_t pPartitionID ) {
    uint64_t allocLength = pAllocLength + STACK_HEADER_LENGTH;
    bit8_t*& partitionStack = mPartitionStacks[pPartitionID];
    bit8_t* partitionHeap = mPartitionHeaps[pPartitionID];

    LLCE_CHECK_ERROR( pAllocLength <= STACK_MAX_ALLOCATION,
        "<<P" << pPartitionID << ">> " <<
        "Cannot allocate an additional stack frame of size " << pAllocLength << "; " <<
        "stack frames are restricted to sizes <= " << STACK_MAX_ALLOCATION << "." );
    LLCE_CHECK_ERROR( partitionHeap < partitionStack - allocLength,
        "<<P" << pPartitionID << ">> " <<
        "Cannot allocate an additional stack frame of size " << pAllocLength << "; " <<
        "stack has only " << partitionStack - partitionHeap << " remaining bytes available." );

    // NOTE(JRC): In order to optimize machine word access, we attempt to align
    // allocated addresses to the machine's word size. See here for details:
    // https://developer.ibm.com/technologies/systems/articles/pa-dalign/
    bit8_t* newStack = partitionStack - allocLength;
    uint64_t newStackOffset = ( newStack - (bit8_t*)0x0 ) % ( 2 * sizeof(size_t) );
    partitionStack = newStack - newStackOffset;
    *(size_t*)partitionStack = allocLength;

    return partitionStack + STACK_HEADER_LENGTH;
}


bit8_t* memory_t::halloc( uint64_t pAllocLength, uint64_t pPartitionID ) {
    // TODO(JRC): Implement this function.
    return nullptr;
}


void memory_t::sfree( uint64_t pPartitionID ) {
    bit8_t*& partitionStack = mPartitionStacks[pPartitionID];

    bit8_t* partitionMax = ( pPartitionID != mPartitionCount - 1 ) ?
        mPartitionBuffers[pPartitionID + 1] : mBuffer + mBufferLength;
    LLCE_CHECK_ERROR( partitionStack != partitionMax,
        "<<P" << pPartitionID << ">> " <<
        "Cannot deallocate an additional stack frame; there are currently " <<
        "no active stack frames in this partition." );

    partitionStack = partitionStack + *(size_t*)partitionStack;
}


void memory_t::hfree( const bit8_t* pAlloc, uint64_t pPartitionID ) {
    // TODO(JRC): Implement this function.
}


bit8_t* memory_t::buffer( uint64_t pPartitionID ) const {
    return mPartitionBuffers[pPartitionID];
}


uint64_t memory_t::length( uint64_t pPartitionID ) const {
    return mPartitionLengths[pPartitionID];
}

}
