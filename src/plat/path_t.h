#ifndef LLCE_PATH_T_H
#define LLCE_PATH_T_H

#include "consts.h"

namespace llce {

namespace platform {

class path_t {
    public:

    /// Class Attributes ///

    // NOTE(JRC): This is the maximum byte length according to the author of eCryptfs.
    // (see: https://unix.stackexchange.com/a/32834)
    const static uint32_t MAX_LENGTH = 4096;

    const static char8_t EOS = '\0';
    const static char8_t DSEP = '/';
    constexpr static char8_t* DUP = nullptr;

    /// Constructors ///

    path_t();
    path_t( const char8_t* pBuffer );
    path_t( const uint32_t pArgCount, ... );

    /// Conversions ///

    operator const char8_t*() const;
    const char8_t* cstr() const;

    /// Class Functions ///

    bool32_t up( const uint32_t pLevels = 1 );
    bool32_t dn( const char8_t* pChild );

    bool32_t exists() const;
    int64_t size() const;
    int64_t modtime() const;

    bool32_t wait() const;

    /// External Functions ///

    friend path_t pathLockPath( const path_t& pBasePath );
    friend path_t exeBasePath();
    friend path_t libFindDLLPath( const char8_t* pLibName );

    private:

    /// Class Fields ///

    char8_t mBuffer[MAX_LENGTH];
    uint32_t mLength;
};

path_t pathLockPath( const path_t& pBasePath );
path_t exeBasePath();
path_t libFindDLLPath( const char8_t* pLibName );

}

}

// NOTE(JRC): This function is unnecessary to define since the implicit
// conversion to the 'char8_t*' type will be used when invoking a C++ output
// operation on the 'llce::platform::path_t' type.
// std::ostream& operator<<( std::ostream& pOS, const hmp::box_t& pBox );

#endif
