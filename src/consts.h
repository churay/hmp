#ifndef LLCE_CONSTS_H
#define LLCE_CONSTS_H

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>

#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

#include "conf.h"

// TODO(JRC): Figure out some way to guarantee that the floating-point types
// will be the correct widths in a platform-independent way.
typedef bool bool8_t;
typedef char bit8_t;
typedef char char8_t;
typedef unsigned char uchar8_t;
typedef uint32_t bool32_t;
typedef float float32_t;
typedef double float64_t;
typedef real real_t;

typedef glm::vec<4, uint8_t, glm::defaultp> vec4u8_t;
typedef glm::vec<2, uint32_t, glm::defaultp> vec2u32_t;
typedef glm::vec<2, float32_t, glm::defaultp> vec2f32_t;
typedef glm::vec<3, float32_t, glm::defaultp> vec3f32_t;
typedef glm::vec<4, float32_t, glm::defaultp> vec4f32_t;

// NOTE(JRC): These can only be passed directly to OpenGL's color array functions
// using '(uint8_t*)&color4u8_t', but this is only because 'GLM_FORCE_XYZW_ONLY' is enabled.
typedef vec4u8_t color4u8_t;
typedef vec4f32_t color4f32_t;

// TODO(JRC): Abstract away the platform-specific code from this module (e.g.
// 'exit', setting the error number, etc.).
// TODO(JRC): Define types that have same byte length as the pointer types
// for the current platform.

// TODO(JRC): Come up with better identifiers for the byte size operators below.
#define KILOBYTE_BL(v) ((v)*1024)
#define MEGABYTE_BL(v) (KILOBYTE_BL(v)*1024)
#define GIGABYTE_BL(v) (MEGABYTE_BL(v)*1024)
#define TERABYTE_BL(v) (GIGABYTE_BL(v)*1024)

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

// TODO(JRC): Consider changing these functions to use 'prinf' functionality
// instead of 'std::cerr' functionality.
// NOTE(JRC): Code inspired by Stack Overflow response:
// https://stackoverflow.com/a/3767883.
#ifdef LLCE_DEBUG
#define LLCE_ALERT_INFO(message) \
    do { \
        std::cerr << "[ALERT]: " << message << std::endl; \
    } while (false)
#else
#define LLCE_ALERT_INFO(message) do { } while(false)
#endif

#ifdef LLCE_DEBUG
#define LLCE_ASSERT_INFO(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "[INFO] (" << __FILE__ << ":" << __LINE__ << "): " << message << std::endl; \
        } \
    } while (false)
#else
#define LLCE_ASSERT_INFO(condition, message) do { } while(false)
#endif

#ifdef LLCE_DEBUG
#define LLCE_ASSERT_DEBUG(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "[DEBUG] (" << __FILE__ << ":" << __LINE__ << "): " << message << std::endl; \
            exit(1); \
        } \
    } while (false)
#else
#define LLCE_ASSERT_DEBUG(condition, message) do { } while(false)
#endif

#define LLCE_ASSERT_ERROR(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "[ERROR] (" << __FILE__ << ":" << __LINE__ << "): " << message << std::endl; \
            exit(1); \
        } \
    } while (false)

#define LLCE_ASSERT_ERRNO() \
    do { \
        if (errno != 0) { \
            std::cerr << "[ERRNO] (" << __FILE__ << ":" << __LINE__ << "): " << strerror( errno ) << std::endl; \
            exit(1); \
        } \
    } while (false)

#endif
