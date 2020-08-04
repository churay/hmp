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

memory_t::memory_t( uint64_t pBufferLength, uint64_t pDataLength, bit8_t* pBufferBase ) :
        mBuffer( nullptr ), mBufferLength( pBufferLength ),
        mData( nullptr ), mDataLength( pDataLength ),
        mHeap( nullptr ), mStack( nullptr ) {
    LLCE_CHECK_ERROR( pDataLength <= pBufferLength,
        "Unable to generate a sensible memory buffer; " <<
        "data segment length " << pDataLength << " exceeds " <<
        "total buffer length " << pBufferLength << "." );

    mBuffer = platform::allocBuffer( mBufferLength, pBufferBase );
    LLCE_CHECK_ERROR( mBuffer != nullptr,
        "Unable to allocate buffer of length " << mBufferLength << " " <<
        "at base address " << pBufferBase << "." );

    // TODO(JRC): Should attempt to word-align at this point to prevent bad
    // alignment for all data accesses.
    mData = mBuffer;
    mHeap = mBuffer + mDataLength;
    mStack = mBuffer + mBufferLength;
}


memory_t::~memory_t() {
    platform::deallocBuffer( mBuffer, mBufferLength );
}


bit8_t* memory_t::dalloc( uint64_t pAllocLength ) {
    uint64_t allocLength = walign( pAllocLength, 0 );
    bit8_t*& allocData = mData;

    LLCE_CHECK_ERROR( pAllocLength <= DATA_MAX_ALLOCATION,
        "Cannot allocate a new data segment of size " << pAllocLength << "; " <<
        "data segments are restricted to sizes <= " << DATA_MAX_ALLOCATION << "." );
    LLCE_CHECK_ERROR( mData + allocLength <= mHeap,
        "Cannot allocate an additional data segment of size " << pAllocLength << "; " <<
        "data has only " << mHeap - mData << " remaining bytes available." );

    allocData += allocLength;

    return allocData - allocLength;
}


bit8_t* memory_t::salloc( uint64_t pAllocLength ) {
    uint64_t allocLength = walign( pAllocLength, STACK_TAG_LENGTH );
    bit8_t*& allocStack = mStack;

    LLCE_CHECK_ERROR( pAllocLength <= STACK_MAX_ALLOCATION,
        "Cannot allocate an additional stack frame of size " << pAllocLength << "; " <<
        "stack frames are restricted to sizes <= " << STACK_MAX_ALLOCATION << "." );
    LLCE_CHECK_ERROR( mHeap <= mStack - allocLength,
        "Cannot allocate an additional stack frame of size " << pAllocLength << "; " <<
        "stack has only " << mStack - mHeap << " remaining bytes available." );

    allocStack -= allocLength;
    *(size_t*)allocStack = static_cast<size_t>( allocLength );

    return allocStack + STACK_TAG_LENGTH;
}


void memory_t::sfree() {
    bit8_t*& allocStack = mStack;

    const bit8_t* cStackMax = mBuffer + mBufferLength;
    LLCE_CHECK_ERROR( allocStack != cStackMax,
        "Cannot deallocate an additional stack frame; there are currently " <<
        "no active stack frames in this partition." );

    allocStack = allocStack + *(size_t*)allocStack;
}


bit8_t* memory_t::halloc( uint64_t pAllocLength ) {
    uint64_t allocLength = walign( pAllocLength, 2 * HEAP_TAG_LENGTH );
    bit8_t*& allocHeap = mHeap;

    LLCE_CHECK_ERROR( pAllocLength <= HEAP_MAX_ALLOCATION,
        "Cannot allocate an additional heap chunk of size " << pAllocLength << "; " <<
        "heap chunks are restricted to sizes <= " << HEAP_MAX_ALLOCATION << "." );

    bit8_t* allocBlock = nullptr;
    for( bit8_t* heapIter = mHeap;
            heapIter != allocHeap || allocBlock != nullptr;
            heapIter += (*(size_t*)heapIter >> 1) ) {
        if( (*(size_t*)heapIter & 0b1) && allocLength <= (*(size_t*)heapIter >> 1) ) {
            allocBlock = heapIter;
        }
    } if( allocBlock == nullptr ) { // create new block
        LLCE_CHECK_ERROR( mHeap + allocLength <= mStack,
            "Cannot allocate an additional heap chunk of size " << pAllocLength << "; " <<
            "heap has only " << mStack - mHeap << " remaining bytes available." );

        size_t blockTag = ( static_cast<size_t>(allocLength) << 1 ) & ~0b1;
        *(size_t*)allocHeap = blockTag;
        *(size_t*)(allocHeap + allocLength - HEAP_TAG_LENGTH) = blockTag;

        allocBlock = allocHeap;
        allocHeap = allocBlock + allocLength;
    }

    *(size_t*)allocBlock &= 0b1;
    *(size_t*)(allocBlock + allocLength - HEAP_TAG_LENGTH) &= 0b1;

    return allocBlock + HEAP_TAG_LENGTH;
}


void memory_t::hfree( bit8_t* pAllocBlock ) {
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

    const bit8_t* cHeapMin = mBuffer + mDataLength;
    const bit8_t* cHeapMax = mStack;
    LLCE_CHECK_ERROR( csIsBoundedBlock(pAllocBlock, cHeapMin, cHeapMax),
        "Cannot deallocate heap memory block at " << pAllocBlock << "; this block " <<
        "lies outside the heap boundaries [" << cHeapMin << ", " << cHeapMax << "]." );
    LLCE_CHECK_ERROR( csIsSoundBlock(pAllocBlock, false),
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

}
