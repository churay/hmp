#include "platform.h"

#include "memory_t.h"

namespace llce {

/// Helper Functions ///

// NOTE(JRC): In order to optimize machine word access, we attempt to align
// allocated addresses to the machine's word size. See here for details:
// https://developer.ibm.com/technologies/systems/articles/pa-dalign/
inline uint64_t walign( uint64_t pBaseLength, uint64_t pHeaderLength ) {
    return pBaseLength + pHeaderLength + ( (pBaseLength + pHeaderLength) % (2 * sizeof(size_t)) );
}

/// Class Functions ///

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
    bit8_t*& partitionStack = mPartitionStacks[pPartitionID];
    bit8_t* partitionHeap = mPartitionHeaps[pPartitionID];
    uint64_t allocLength = walign( pAllocLength, STACK_TAG_LENGTH );

    LLCE_CHECK_ERROR( pAllocLength <= STACK_MAX_ALLOCATION,
        "<<P" << pPartitionID << ">> " <<
        "Cannot allocate an additional stack frame of size " << pAllocLength << "; " <<
        "stack frames are restricted to sizes <= " << STACK_MAX_ALLOCATION << "." );
    LLCE_CHECK_ERROR( partitionHeap < partitionStack - allocLength,
        "<<P" << pPartitionID << ">> " <<
        "Cannot allocate an additional stack frame of size " << pAllocLength << "; " <<
        "stack has only " << partitionStack - partitionHeap << " remaining bytes available." );

    partitionStack -= allocLength;
    *(size_t*)partitionStack = static_cast<size_t>( allocLength );

    return partitionStack + STACK_TAG_LENGTH;
}


void memory_t::sfree( uint64_t pPartitionID ) {
    bit8_t*& partitionStack = mPartitionStacks[pPartitionID];

    const bit8_t* cStackMax = ( pPartitionID != mPartitionCount - 1 ) ?
        mPartitionBuffers[pPartitionID + 1] : mBuffer + mBufferLength;
    LLCE_CHECK_ERROR( partitionStack != cStackMax,
        "<<P" << pPartitionID << ">> " <<
        "Cannot deallocate an additional stack frame; there are currently " <<
        "no active stack frames in this partition." );

    partitionStack = partitionStack + *(size_t*)partitionStack;
}


bit8_t* memory_t::halloc( uint64_t pAllocLength, uint64_t pPartitionID ) {
    bit8_t*& partitionHeap = mPartitionHeaps[pPartitionID];
    bit8_t* partitionStack = mPartitionStacks[pPartitionID];
    uint64_t allocLength = walign( pAllocLength, 2 * HEAP_TAG_LENGTH );

    LLCE_CHECK_ERROR( pAllocLength <= HEAP_MAX_ALLOCATION,
        "<<P" << pPartitionID << ">> " <<
        "Cannot allocate an additional heap chunk of size " << pAllocLength << "; " <<
        "heap chunks are restricted to sizes <= " << HEAP_MAX_ALLOCATION << "." );

    bit8_t* allocBlock = nullptr;
    for( bit8_t* heapIter = mPartitionBuffers[pPartitionID];
            heapIter != partitionHeap || allocBlock != nullptr;
            heapIter += (*(size_t*)heapIter >> 1) ) {
        if( (*(size_t*)heapIter & 0b1) && allocLength <= (*(size_t*)heapIter >> 1) ) {
            allocBlock = heapIter;
        }
    } if( allocBlock == nullptr ) { // create new block
        LLCE_CHECK_ERROR( partitionHeap + allocLength < partitionStack,
            "<<P" << pPartitionID << ">> " <<
            "Cannot allocate an additional heap chunk of size " << pAllocLength << "; " <<
            "heap has only " << partitionStack - partitionHeap << " remaining bytes available." );

        size_t blockTag = ( static_cast<size_t>(allocLength) << 1 ) & ~0b1;
        *(size_t*)partitionHeap = blockTag;
        *(size_t*)(partitionHeap + allocLength - HEAP_TAG_LENGTH) = blockTag;

        allocBlock = partitionHeap;
        partitionHeap = allocBlock + allocLength;
    }

    *(size_t*)allocBlock &= 0b1;
    *(size_t*)(allocBlock + allocLength - HEAP_TAG_LENGTH) &= 0b1;

    return allocBlock + HEAP_TAG_LENGTH;
}


void memory_t::hfree( bit8_t* pAllocBlock, uint64_t pPartitionID ) {
    const static auto csIsBoundedBlock = []
            ( bit8_t* pBlock, const bit8_t* pPartMin, const bit8_t* pPartMax ) {
        return pPartMin < pBlock && pBlock <pPartMax;
    };
    const static auto csIsSoundBlock = [] ( bit8_t* pBlock, bool8_t pIsEnd ) {
        const size_t cBlockMinTag = *(size_t*)(pBlock - HEAP_TAG_LENGTH);
        const uint64_t cBlockLength = cBlockMinTag >> 1;
        const bool8_t cBlockIsOccupied = cBlockMinTag & 0b1;
        const size_t cBlockMaxTag = *(size_t*)(pBlock + (pIsEnd ?
            (-cBlockLength + HEAP_TAG_LENGTH) :
            (cBlockLength - 2 * HEAP_TAG_LENGTH)));
        return ( cBlockMinTag == cBlockMaxTag ) && cBlockIsOccupied;
    };

    const bit8_t* cHeapMin = mPartitionBuffers[pPartitionID];
    const bit8_t* cHeapMax = mPartitionStacks[pPartitionID];
    LLCE_CHECK_ERROR( csIsBoundedBlock(pAllocBlock, cHeapMin, cHeapMax),
        "<<P" << pPartitionID << ">> " <<
        "Cannot deallocate heap memory block at " << pAllocBlock << "; this block " <<
        "lies outside the heap boundaries [" << cHeapMin << ", " << cHeapMax << "]." );
    LLCE_CHECK_ERROR( csIsSoundBlock(pAllocBlock, false),
        "<<P" << pPartitionID << ">> " <<
        "Cannot deallocate heap memory block at " << pAllocBlock << "; this block " <<
        "is either unmanaged by the heap, has been freed before, or has been overwritten." );

    const size_t cAllocBlockSize = *(size_t*)(pAllocBlock - HEAP_TAG_LENGTH) >> 1;

    bit8_t* freedBlock = pAllocBlock;
    size_t freedBlockSize = cAllocBlockSize; {
        bit8_t* prevBlock = pAllocBlock - HEAP_TAG_LENGTH;
        if( csIsBoundedBlock(prevBlock, cHeapMin, cHeapMax) && csIsSoundBlock(prevBlock, true) ) {
            const size_t cPrevBlockSize = *(size_t*)(prevBlock - HEAP_TAG_LENGTH) >> 1;
            freedBlock = prevBlock - cPrevBlockSize + HEAP_TAG_LENGTH;
            freedBlockSize += cPrevBlockSize;
        }

        bit8_t* nextBlock = pAllocBlock + cAllocBlockSize;
        if( csIsBoundedBlock(nextBlock, cHeapMin, cHeapMax) && csIsSoundBlock(nextBlock, false) ) {
            freedBlockSize += *(size_t*)(nextBlock - HEAP_TAG_LENGTH) >> 1;
        }
    }

    size_t blockTag = ( freedBlockSize << 1 ) & ~0b1;
    *(size_t*)freedBlock = blockTag;
    *(size_t*)(freedBlock + freedBlockSize - HEAP_TAG_LENGTH) = blockTag;
}


bit8_t* memory_t::buffer( uint64_t pPartitionID ) const {
    return mPartitionBuffers[pPartitionID];
}


uint64_t memory_t::length( uint64_t pPartitionID ) const {
    return mPartitionLengths[pPartitionID];
}

}
