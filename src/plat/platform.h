#ifndef LLCE_PLATFORM_H
#define LLCE_PLATFORM_H

#include "consts.h"
#include "conf.h"

namespace llce {

namespace platform {
    /// Namespace Functions ///

    bit8_t* allocBuffer( uint64_t pBufferLength, bit8_t* pBufferStart = nullptr );
    bool32_t deallocBuffer( bit8_t* pBuffer, uint64_t pBufferLength );

    void* dllLoadHandle( const char8_t* pDLLPath );
    bool32_t dllUnloadHandle( void* pDLLHandle, const char8_t* pDLLPath );
    void* dllLoadSymbol( void* pDLLHandle, const char8_t* pDLLSymbol );

    bool32_t pngSave( const char8_t* pPNGPath, const bit8_t* pPNGData, const uint32_t& pPNGWidth, const uint32_t& pPNGHeight );
    bool32_t pngLoad( const char8_t* pPNGPath, bit8_t* pPNGData, uint32_t& pPNGWidth, uint32_t& pPNGHeight );
}

}

#endif
