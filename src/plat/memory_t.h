#ifndef LLCE_MEMORY_T_H
#define LLCE_MEMORY_T_H

#include "consts.h"

namespace llce {

class memory_t {
    public:

    /// Class Attributes ///

    const static uint32_t MAX_PARTITIONS = 8;
    const static uint32_t MAX_STACK_SIZE = 1024;

    /// Constructors ///

    memory_t( uint64_t pBufferLength, bit8_t* pBufferBase = nullptr );
    memory_t( const uint64_t* pPartitionLengths, uint64_t pPartitionCount, bit8_t* pBufferBase = nullptr );
    ~memory_t();

    /// Class Functions ///

    bit8_t* salloc( uint64_t pAllocLength, uint64_t pPartitionID = 0 );
    // bit8_t* halloc( uint64_t pAllocLength, uint64_t pPartitionID = 0 );

    void sfree( uint64_t pPartitionID = 0 );
    // void hfree( const bit8_t* pAlloc );

    bit8_t* buffer( uint64_t pPartitionID = 0 ) const;
    uint64_t length( uint64_t pPartitionID = 0 ) const;

    private:

    /// Class Fields ///

    bit8_t* mBuffer;
    uint64_t mBufferLength;

    uint64_t mPartitionCount;
    bit8_t* mPartitionBuffers[MAX_PARTITIONS];
    uint64_t mPartitionLengths[MAX_PARTITIONS];

    uint64_t mPartitionStackIndices[MAX_PARTITIONS];
    uint64_t mPartitionStackAllocs[MAX_PARTITIONS][MAX_STACK_SIZE];

    uint64_t mPartitionHallocs[MAX_PARTITIONS];
    // TODO(JRC): Need more information for 'mallocs' to work.
};

}

#endif
