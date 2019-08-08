#include <string.h>
#include <stdio.h>

#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>

#include "platform.h"

#ifdef LLCE_CAPTURE
extern "C" {
#include <png.h>
}
#endif

namespace llce {

// NOTE(JRC): Documentation on the buffer allocation functions on Linux can be
// found here: http://man7.org/linux/man-pages/man2/mmap.2.html

bit8_t* platform::allocBuffer( uint64_t pBufferLength, bit8_t* pBufferBase ) {
    const int64_t cPermissionFlags = PROT_READ | PROT_WRITE;
    const int64_t cAllocFlags = MAP_ANONYMOUS | MAP_PRIVATE |
        ( (pBufferBase != nullptr) ? MAP_FIXED : 0 );

    // TODO(JRC): This whole piece is a bit messy and prone to race conditions
    // and thus should be cleaned up if possible.
    if( cAllocFlags & MAP_FIXED ) {
        const int64_t cPageSize = sysconf( _SC_PAGESIZE );
        uchar8_t mincoreBuffer = false;

        bool32_t isBufferOccupied = false;
        for( bit8_t* pageIt = pBufferBase; pageIt < pBufferBase + pBufferLength; pageIt += cPageSize ) {
            isBufferOccupied |= !(
                mincore( pageIt, cPageSize, &mincoreBuffer ) == -1 &&
                errno == ENOMEM );
        }

        LLCE_ASSERT_INFO( !isBufferOccupied,
            "Allocation of buffer of length " << pBufferLength << " " <<
            "at base address " << pBufferBase << " will cause eviction of one "
            "or more existing memory blocks." );
    }

    bit8_t* buffer = (bit8_t*)mmap(
        pBufferBase,             // Memory Start Address
        pBufferLength,           // Allocation Length (Bytes)
        cPermissionFlags,        // Data Permission Flags (Read/Write)
        cAllocFlags,             // Map Options (In-Memory, Private to Process)
        -1,                      // File Descriptor
        0 );                     // File Offset

    LLCE_ASSERT_INFO( buffer != (bit8_t*)MAP_FAILED,
        "Unable to allocate buffer of length " << pBufferLength << " " <<
        "at base address " << pBufferBase << "; " << strerror(errno) );

    return buffer;
}


bool32_t platform::deallocBuffer( bit8_t* pBuffer, uint64_t pBufferLength ) {
    int64_t status = munmap( pBuffer, pBufferLength );

    LLCE_ASSERT_INFO( status == 0,
        "Deallocation of buffer of length " << pBufferLength << " " <<
        "at base address " << pBuffer << " failed; possible memory corruption." );

    return status == 0;
}

// NOTE(JRC): Documentation on Linux's dynamic-library loading functions can be
// found here: http://man7.org/linux/man-pages/man3/dlmopen.3.html

void* platform::dllLoadHandle( const char8_t* pDLLPath ) {
    void* libraryHandle = dlopen( pDLLPath, RTLD_NOW | RTLD_GLOBAL );
    const char8_t* libraryError = dlerror();

    LLCE_ASSERT_INFO( libraryHandle != nullptr,
        "Failed to load library `" << pDLLPath << "`: " << libraryError );

    return libraryHandle;
}


bool32_t platform::dllUnloadHandle( void* pDLLHandle, const char8_t* pDLLPath ) {
    int64_t status = dlclose( pDLLHandle );
    const char8_t* libraryError = dlerror();

    LLCE_ASSERT_INFO( status == 0,
        "Failed to unload library `" << pDLLPath << "`; " << libraryError );

    return status == 0;
}


void* platform::dllLoadSymbol( void* pDLLHandle, const char8_t* pDLLSymbol ) {
    void* symbolFunction = dlsym( const_cast<void*>(pDLLHandle), pDLLSymbol );
    const char8_t* symbolError = dlerror();

    LLCE_ASSERT_INFO( symbolFunction != nullptr,
        "Failed to load symbol `" << pDLLSymbol << "`: " << symbolError );

    return symbolFunction;
}

#ifdef LLCE_CAPTURE

// NOTE(JRC): Code below heavily inspired by GitHub gist of user 'niw':
// https://gist.github.com/niw/5963798

bool32_t platform::pngSave( const char8_t* pPNGPath, const bit8_t* pPNGData, const uint32_t& pPNGWidth, const uint32_t& pPNGHeight ) {
    bool32_t saveSuccessful = false;

    FILE* pngFile = nullptr;
    LLCE_ASSERT_INFO( (pngFile = fopen(pPNGPath, "wb")) != nullptr,
        "Failed to open PNG file at path '" << pPNGPath << "'." );

    // TODO(JRC): For local memory allocation handling, use png_create_write_struct_2.
    png_struct* pngBase = nullptr;
    LLCE_ASSERT_INFO(
        (pngBase = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)) != nullptr,
        "Failed to create base headers for PNG file at path '" << pPNGPath << "'." );
    png_info* pngInfo = nullptr;
    LLCE_ASSERT_INFO(
        (pngInfo = png_create_info_struct(pngBase)) != nullptr,
        "Failed to create info headers for PNG file at path '" << pPNGPath << "'." );

    if( pngFile != nullptr && pngBase != nullptr && pngInfo != nullptr ) {
        png_init_io( pngBase, pngFile );
        png_set_IHDR(
            pngBase, pngInfo, pPNGWidth, pPNGHeight,
            8, PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );

        png_write_info( pngBase, pngInfo );
        for( uint32_t rowIdx = 0; rowIdx < pPNGHeight; rowIdx++ ) {
            png_write_row( pngBase, (png_byte*)&pPNGData[rowIdx * pPNGWidth * 4] );
        }
        png_write_end( pngBase, nullptr );

        png_destroy_write_struct( &pngBase, &pngInfo );
        fclose( pngFile );

        saveSuccessful = true;
    }

    return saveSuccessful;
}


bool32_t platform::pngLoad( const char8_t* pPNGPath, bit8_t* pPNGData, uint32_t& pPNGWidth, uint32_t& pPNGHeight ) {
    bool32_t loadSuccessful = false;
    pPNGWidth = pPNGHeight = 0;

    FILE* pngFile = nullptr;
    LLCE_ASSERT_INFO( (pngFile = fopen(pPNGPath, "rb")) != nullptr,
        "Failed to open PNG file at path '" << pPNGPath << "'." );

    // TODO(JRC): For local memory allocation handling, use png_create_read_struct_2.
    png_struct* pngBase = nullptr;
    LLCE_ASSERT_INFO(
        (pngBase = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)) != nullptr,
        "Failed to create base headers for PNG file at path '" << pPNGPath << "'." );
    png_info* pngInfo = nullptr;
    LLCE_ASSERT_INFO(
        (pngInfo = png_create_info_struct(pngBase)) != nullptr,
        "Failed to create info headers for PNG file at path '" << pPNGPath << "'." );

    if( pngFile != nullptr && pngBase != nullptr && pngInfo != nullptr ) {
        png_init_io( pngBase, pngFile );
        png_read_info( pngBase, pngInfo );

        pPNGWidth = png_get_image_width( pngBase, pngInfo );
        pPNGHeight = png_get_image_height( pngBase, pngInfo );
        const uint32_t cPNGColorType = png_get_color_type( pngBase, pngInfo );
        const uint32_t cPNGBitDepth = png_get_bit_depth( pngBase, pngInfo );

        { // Convert 'Color Type' to RGBA8 //
            if( cPNGBitDepth == 16 ) {
                png_set_strip_16( pngBase );
            } if( cPNGColorType == PNG_COLOR_TYPE_PALETTE ) {
                png_set_palette_to_rgb( pngBase );
            } if( cPNGColorType == PNG_COLOR_TYPE_GRAY && cPNGBitDepth < 8 ) {
                png_set_expand_gray_1_2_4_to_8( pngBase );
            } if( png_get_valid(pngBase, pngInfo, PNG_INFO_tRNS) ) {
                png_set_tRNS_to_alpha( pngBase );
            } if( cPNGColorType == PNG_COLOR_TYPE_RGB ||
                    cPNGColorType == PNG_COLOR_TYPE_GRAY ||
                    cPNGColorType == PNG_COLOR_TYPE_PALETTE ) {
                png_set_filler( pngBase, 0xFF, PNG_FILLER_AFTER );
            } if( cPNGColorType == PNG_COLOR_TYPE_GRAY ||
                    cPNGColorType == PNG_COLOR_TYPE_GRAY_ALPHA ) {
                png_set_gray_to_rgb( pngBase );
            }

            png_read_update_info( pngBase, pngInfo );
        }

        for( uint32_t rowIdx = 0; rowIdx < pPNGHeight; rowIdx++ ) {
            png_read_row( pngBase, (png_byte*)&pPNGData[rowIdx * pPNGWidth * 4], nullptr );
        }

        png_destroy_read_struct( &pngBase, &pngInfo, nullptr );
        fclose( pngFile );

        loadSuccessful = true;
    }

    return loadSuccessful;
}

#endif

}
