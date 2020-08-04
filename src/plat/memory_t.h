#ifndef LLCE_MEMORY_T_H
#define LLCE_MEMORY_T_H

#include <limits>

#include "consts.h"

namespace llce {

class memory_t {
    public:

    /// Class Attributes ///

    const static uint64_t STACK_TAG_LENGTH = sizeof(size_t);
    const static uint64_t HEAP_TAG_LENGTH = sizeof(size_t);

    const static uint64_t MAX_ALLOCATION_OFFSET = sizeof(size_t) - 1;
    const static uint64_t DATA_MAX_ALLOCATION = std::numeric_limits<size_t>::max();
    const static uint64_t STACK_MAX_ALLOCATION =
        std::numeric_limits<size_t>::max() - STACK_TAG_LENGTH - MAX_ALLOCATION_OFFSET;
    const static uint64_t HEAP_MAX_ALLOCATION =
        (std::numeric_limits<size_t>::max() >> 1) - 2 * HEAP_TAG_LENGTH - MAX_ALLOCATION_OFFSET;

    /// Constructors ///

    memory_t( uint64_t pBufferLength, uint64_t pDataLength, bit8_t* pBufferBase = nullptr );
    ~memory_t();

    /// Class Functions ///

    bit8_t* dalloc( uint64_t pAllocLength );
    // void dfree();

    bit8_t* salloc( uint64_t pAllocLength );
    void sfree();

    bit8_t* halloc( uint64_t pAllocLength );
    void hfree( bit8_t* pAllocBlock );

    inline bit8_t* buffer() const { return mBuffer; }
    inline uint64_t length() const { return mBufferLength; }

    private:

    /// Class Fields ///

    bit8_t* mBuffer;
    uint64_t mBufferLength;

    bit8_t* mData;
    uint64_t mDataLength;

    bit8_t* mHeap;
    bit8_t* mStack;
};

}

#endif
