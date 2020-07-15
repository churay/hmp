#include "platform.h"

#include "memory_t.h"

namespace llce {

memory_t::memory_t( uint64_t pBufferLength, bit8_t* pBufferBase ) :
        memory_t( &pBufferLength, 1, pBufferBase ) {
    
}


memory_t::memory_t( const uint64_t* pPartitionLengths, uint64_t pPartitionCount, bit8_t* pBufferBase ) {
    LLCE_CHECK_ERROR( pPartitionCount <= memory_t::MAX_PARTITIONS,
        "Couldn't allocate memory chunk with " << pPartitionCount << " blocks; " <<
        "block count has a maximum value of " << memory_t::MAX_PARTITIONS << "." );

    mPartitionCount = pPartitionCount;
    mBufferLength = 0;
    for( uint64_t blockIdx = 0; blockIdx < pPartitionCount; blockIdx++ ) {
        mPartitionLengths[blockIdx] = pPartitionLengths[blockIdx];
        mPartitionSallocs[blockIdx] = 0;
        mPartitionMallocs[blockIdx] = 0;
        mBufferLength += pPartitionLengths[blockIdx];
    }

    mBuffer = platform::allocBuffer( mBufferLength, pBufferBase );
    LLCE_CHECK_ERROR( mBuffer != nullptr,
        "Unable to allocate buffer of length " << mBufferLength << " " <<
        "at base address " << pBufferBase << ".");

    mPartitionBuffers[0] = mBuffer;
    for( uint64_t blockIdx = 1; blockIdx < pPartitionCount; blockIdx++ ) {
        mPartitionBuffers[blockIdx] = mPartitionBuffers[blockIdx-1] + mPartitionLengths[blockIdx-1];
    }
}


memory_t::~memory_t() {
    platform::deallocBuffer( mBuffer, mBufferLength );
}


bit8_t* memory_t::salloc( uint64_t pAllocLength, uint64_t pPartitionIdx ) {
    bit8_t* partitionBuffer = mPartitionBuffers[(pPartitionIdx < mPartitionCount) ? pPartitionIdx : 0];
    uint64_t partitionLength = mPartitionLengths[(pPartitionIdx < mPartitionCount) ? pPartitionIdx : 0];
    uint64_t& partitionAlloc = mPartitionSallocs[(pPartitionIdx < mPartitionCount) ? pPartitionIdx : 0];

    LLCE_CHECK_ERROR( partitionAlloc + pAllocLength <= partitionLength,
        "Cannot allocate an additional " << pAllocLength << " bytes to buffer " <<
        pPartitionIdx << "; allocation puts block over " << partitionAlloc <<
        "/" << partitionLength << " allocation capacity." );

    bit8_t* allocBuffer = partitionBuffer + partitionAlloc;
    partitionAlloc += pAllocLength;
    return allocBuffer;
}


bit8_t* memory_t::malloc( uint64_t pAllocLength, uint64_t pPartitionIdx ) {
    // TODO(JRC): Implement this function.
    return nullptr;
}


bit8_t* memory_t::buffer( uint64_t pPartitionIdx ) const {
    return mPartitionBuffers[(pPartitionIdx < mPartitionCount) ? pPartitionIdx : 0];
}


uint64_t memory_t::length( uint64_t pPartitionIdx ) const {
    return mPartitionLengths[(pPartitionIdx < mPartitionCount) ? pPartitionIdx : 0];
}

}
