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

    mPartitionCount = pPartitionCount;
    mBufferLength = 0;
    for( uint64_t partitionIdx = 0; partitionIdx < pPartitionCount; partitionIdx++ ) {
        mPartitionLengths[partitionIdx] = pPartitionLengths[partitionIdx];
        mPartitionStackIndices[partitionIdx] = 0;
        mPartitionStackAllocs[partitionIdx][0] = 0;
        mBufferLength += pPartitionLengths[partitionIdx];
    }

    mBuffer = platform::allocBuffer( mBufferLength, pBufferBase );
    LLCE_CHECK_ERROR( mBuffer != nullptr,
        "Unable to allocate buffer of length " << mBufferLength << " " <<
        "at base address " << pBufferBase << ".");

    mPartitionBuffers[0] = mBuffer;
    for( uint64_t partitionIdx = 1; partitionIdx < pPartitionCount; partitionIdx++ ) {
        mPartitionBuffers[partitionIdx] = mPartitionBuffers[partitionIdx-1] + mPartitionLengths[partitionIdx-1];
    }
}


memory_t::~memory_t() {
    platform::deallocBuffer( mBuffer, mBufferLength );
}


bit8_t* memory_t::salloc( uint64_t pAllocLength, uint64_t pPartitionIdx ) {
    pPartitionIdx = ( pPartitionIdx < mPartitionCount ) ? pPartitionIdx : 0;

    bit8_t* partitionBuffer = mPartitionBuffers[pPartitionIdx];
    uint64_t partitionLength = mPartitionLengths[pPartitionIdx];
    uint64_t* stackPointer = &mPartitionStackAllocs[pPartitionIdx][0];
    uint64_t& stackIndex = mPartitionStackIndices[pPartitionIdx];

    LLCE_CHECK_ERROR( stackPointer[stackIndex] + pAllocLength <= partitionLength,
        "Cannot allocate an additional " << pAllocLength << " bytes to buffer " <<
        pPartitionIdx << "; allocation puts partition over " << stackPointer[stackIndex] <<
        "/" << partitionLength << " allocation capacity." );

    stackPointer[stackIndex + 1] = stackPointer[stackIndex] + pAllocLength;
    return partitionBuffer + stackPointer[stackIndex++];
}


void memory_t::sfree( uint64_t pPartitionIdx ) {
    // TODO(JRC): Implement this function.
}


bit8_t* memory_t::buffer( uint64_t pPartitionIdx ) const {
    return mPartitionBuffers[(pPartitionIdx < mPartitionCount) ? pPartitionIdx : 0];
}


uint64_t memory_t::length( uint64_t pPartitionIdx ) const {
    return mPartitionLengths[(pPartitionIdx < mPartitionCount) ? pPartitionIdx : 0];
}

}
