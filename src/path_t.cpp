#include <cstdarg>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <elf.h>
#include <link.h>

#include "path_t.h"

namespace llce {

namespace platform {

/// Class Functions ///

path_t::path_t() {
    mLength = 0;
    mBuffer[mLength] = path_t::EOS;
}


path_t::path_t( const char8_t* pBuffer ) {
    uint32_t inLength = 0;
    for( const char8_t* pItr = pBuffer; *pItr != path_t::EOS; pItr++, inLength++ ) {}

    memcpy( &mBuffer[0], pBuffer, inLength );
    mLength = inLength;
    mBuffer[mLength] = path_t::EOS;
}


path_t::path_t( const uint32_t pArgCount, ... ) {
    mLength = 0;
    mBuffer[mLength] = path_t::EOS;

    va_list args;
    va_start( args, pArgCount );

    bool32_t areArgsValid = true;
    
    for( uint32_t argIdx = 0; argIdx < pArgCount; argIdx++ ) {
        const char8_t* arg = va_arg( args, const char8_t* );

        if( argIdx == 0 ) {
            uint32_t inLength = 0;
            for( const char8_t* pItr = arg; *pItr != path_t::EOS; pItr++, inLength++ ) {}

            memcpy( &mBuffer[0], arg, inLength );
            mLength = inLength;
            mBuffer[mLength] = path_t::EOS;
        } else {
            areArgsValid &= ( arg == path_t::DUP ) ? up() : dn( arg );
        }
    }

    va_end( args );

    LLCE_ASSERT_INFO( areArgsValid,
        "Failed to initialize arbitrary variadic path; one or more indicated " <<
        "arguments were invalid." );
}


path_t::operator const char8_t*() const {
    return &mBuffer[0];
}


const char8_t* path_t::cstr() const {
    return &mBuffer[0];
}


bool32_t path_t::up( const uint32_t pLevels ) {
    bool32_t success = true;

    char8_t* pathItr = nullptr;
    for( pathItr = &mBuffer[0]; *pathItr != path_t::EOS; pathItr++ ) {}

    for( uint32_t levelIdx = 0; levelIdx < pLevels; levelIdx++ ) {
        for( ; pathItr > &mBuffer[0] && *pathItr != path_t::DSEP; pathItr-- ) {}

        bool32_t hasPathParent = pathItr > &mBuffer[0];
        success &= hasPathParent;
        if( !hasPathParent ) {
            LLCE_ASSERT_INFO( false,
                "Cannot find ancestor at level " << (levelIdx + 1) <<
                " for path `" << &mBuffer[0] << "`." );
            break;
        } else {
            *pathItr = path_t::EOS;
            mLength = pathItr - &mBuffer[0];
        }
    }

    return success;
}


bool32_t path_t::dn( const char8_t* pChild ) {
    bool32_t success = true;

    uint32_t childLength = 0;
    for( const char8_t* pItr = pChild; *pItr != path_t::EOS; pItr++, childLength++ ) {}

    bool32_t isPathOverflowed = mLength + childLength + 1 > path_t::MAX_LENGTH;
    success &= !isPathOverflowed;

    if( isPathOverflowed ) {
        LLCE_ASSERT_INFO( false,
            "Cannot find child `" << pChild << "` of extended path `" << &mBuffer[0] << "`." );
    } else {
        mBuffer[mLength] = path_t::DSEP;
        memcpy( &mBuffer[mLength + 1], pChild, childLength );
        mLength += 1 + childLength;
        mBuffer[mLength] = path_t::EOS;
    }

    return success;
}


bool32_t path_t::exists() const {
    return !access( &mBuffer[0], F_OK );
}


int64_t path_t::size() const {
    int64_t fileSize = 0;

    // NOTE(JRC): According to the Unix documentation, the 'off_t' type is
    // flexible, but should be some form of signed integer.
    // (see: http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_types.h.html#tag_13_67)
    struct stat fileStatus;
    if( !stat(&mBuffer[0], &fileStatus) ) {
        fileSize = static_cast<int64_t>( fileStatus.st_size );
    }

    LLCE_ASSERT_INFO( fileSize > 0,
        "Failed to read size of file " << &mBuffer[0] << "; " <<
        strerror(errno) );

    return fileSize;
}


int64_t path_t::modtime() const {
    int64_t fileModTime = 0;

    // NOTE(JRC): According to the C++ documentation, the 'time_t' type is
    // defined to be a 32-bit signed integer by most Unix implementations.
    // (see: http://en.cppreference.com/w/c/chrono/time)
    struct stat fileStatus;
    if( !stat(&mBuffer[0], &fileStatus) ) {
        fileModTime = static_cast<int64_t>( fileStatus.st_mtime );
    }

    LLCE_ASSERT_INFO( fileModTime > 0,
        "Failed to read mod time of file at path " << &mBuffer[0] << "; " <<
        strerror(errno) );

    return fileModTime;
}


bool32_t path_t::wait() const {
    bool32_t waitSuccessful = false;

    int32_t fileHandle;
    if( (fileHandle = open(&mBuffer[0], O_RDWR)) >= 0 ) {
        // NOTE(JRC): The 'flock' function will block on a file if it has been
        // f'locked by another process; it's used here to wait on the flock and
        // then immediately continue processing.
        waitSuccessful = !flock( fileHandle, LOCK_EX ) && !flock( fileHandle, LOCK_UN );
        waitSuccessful &= !close( fileHandle );
    }

    LLCE_ASSERT_INFO( waitSuccessful,
        "Failed to wait for file at path " << &mBuffer[0] << "; " <<
        strerror(errno) );

    return waitSuccessful;
}

/// External Functions ///

path_t pathLockPath( const path_t& pBasePath ) {
    const char8_t* cLockSuffix = ".lock";

    path_t lockPath = pBasePath;
    for( const char8_t* pItr = cLockSuffix; *pItr != path_t::EOS; pItr++ ) {
        lockPath.mBuffer[lockPath.mLength++] = *pItr;
    }
    lockPath.mBuffer[lockPath.mLength] = path_t::EOS;

    return lockPath;
}


path_t exeBasePath() {
    path_t exePath;
    int64_t status = readlink( "/proc/self/exe", &exePath.mBuffer[0],
        path_t::MAX_LENGTH );

    int64_t pathLength = ( status <= 0 ) ? 0 : status;
    LLCE_ASSERT_INFO( pathLength > 0,
        "Failed to retrieve the path to the running executable; " <<
        strerror(errno) );

    // NOTE(JRC): It's important that the null character be set explicitly
    // since 'readlink' doesn't guarantee this behavior by default (see:
    // http://man7.org/linux/man-pages/man2/readlink.2.html).
    exePath.mLength = pathLength;
    exePath.mBuffer[pathLength] = path_t::EOS;

    return exePath;
}


path_t libFindDLLPath( const char8_t* pLibName ) {
    path_t libPath;

    // NOTE(JRC): The contents of this function heavily reference the system
    // implementation of the '<link.h>' dependency, which defines the C data
    // structures that interface with dynamic library symbol tables.
    // TODO(JRC): Extend this solution so that it loads using 'DT_RUNPATH' and
    // '$ORIGIN' like the built-in Unix run-time loading mechanism does.
    const char8_t* procStringTable = nullptr;
    int32_t procRPathOffset = -1;

    for( const ElfW(Dyn)* dylibIter = _DYNAMIC; dylibIter->d_tag != DT_NULL; ++dylibIter ) {
        if( dylibIter->d_tag == DT_STRTAB ) {
            procStringTable = (const char8_t*)( dylibIter->d_un.d_val );
        } else if( dylibIter->d_tag == DT_RPATH ) {
            procRPathOffset = (int32_t)( dylibIter->d_un.d_val );
        }
    }

    const char8_t* procRPath = ( procStringTable != nullptr && procRPathOffset >= 0 ) ?
        procStringTable + procRPathOffset : nullptr;
    for( const char8_t* pathIter = procRPath;
            pathIter != nullptr && *pathIter != path_t::EOS && !libPath.exists();
            pathIter = strchr(pathIter, ':') ) {
        libPath.mLength = 0;

        for( const char8_t* pItr = pathIter; *pItr != path_t::EOS; pItr++ ) {
            libPath.mBuffer[libPath.mLength++] = *pItr;
        }
        libPath.mBuffer[libPath.mLength++] = path_t::DSEP;
        for( const char8_t* pItr = pLibName; *pItr != path_t::EOS; pItr++ ) {
            libPath.mBuffer[libPath.mLength++] = *pItr;
        }

        libPath.mBuffer[libPath.mLength] = path_t::EOS;
    }

    LLCE_ASSERT_INFO( libPath.exists(),
        "Failed to find `" << pLibName << "` in the executable's dynamic path; " <<
        strerror(errno) );

    return libPath;
}

}

}
