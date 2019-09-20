#ifndef LLCE_BUFFER_T_H
#define LLCE_BUFFER_T_H

#include "consts.h"

namespace llce {

class buffer_t {
    public:

    /// Constructors ///

    buffer_t( bit8_t* pBuffer, const uint64_t pBufferCapacity );

    /// Class Functions ///

    bool32_t enqueue( const bit8_t* pData, const uint64_t pDataLength );
    bool32_t dequeue( bit8_t* pData = nullptr, const uint64_t pDataLength = 0 );
    bool32_t clear();

    uint64_t length() const;

    private:

    /// Class Fields ///

    bit8_t* mBuffer;
    uint64_t mBufferCapacity;

    uint64_t mBufferStart;
    uint64_t mBufferEnd;
};

}

#endif
