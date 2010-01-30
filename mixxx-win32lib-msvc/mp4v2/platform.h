#ifndef MP4V2_PLATFORM_H
#define MP4V2_PLATFORM_H

/*****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

#if defined( _WIN32 ) && !defined( __MINGW32__ )
    typedef char      int8_t;
    typedef short     int16_t;
    typedef int       int32_t;
    typedef long long int64_t;

    typedef unsigned char      uint8_t;
    typedef unsigned short     uint16_t;
    typedef unsigned int       uint32_t;
    typedef unsigned long long uint64_t;
#else
#   include <stdint.h>
#endif

#if defined( _WIN32 ) || defined( __MINGW32__ )
#   if defined( _WINDLL ) || defined( DLL_EXPORT )
#       define MP4V2_EXPORT __declspec(dllexport)
#   elif defined( _DLL ) || defined( DLL_IMPORT ) 
#       define MP4V2_EXPORT __declspec(dllimport)
#   else
#       define MP4V2_EXPORT
#   endif
#else
#   define MP4V2_EXPORT __attribute__((visibility("default")))
#endif

#if defined( __GNUC__ )
#   define MP4V2_DEPRECATED __attribute__((deprecated))
#else
#   define MP4V2_DEPRECATED
#endif

/******************************************************************************
 *
 * TODO-KB: cleanup -- absolutely no need for a C-API to fuss with reserved
 * C++ keywords. This will involve changing the public interface and current
 * plan of action:
 *
 *      typdef enum {
 *          mp4_false,
 *          mp4_true,
 *      } mp4_bool_t;
 *
 * followed by updating all public signatures and implementation.
 */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#if !defined( __cplusplus )
#ifndef bool
#if SIZEOF_BOOL == 8
typedef uint64_t bool;
#else
#if SIZEOF_BOOL == 4
typedef uint32_t bool;
#else
#if SIZEOF_BOOL == 2
typedef uint16_t bool;
#else
typedef unsigned char bool;
#endif
#endif
#endif
#ifndef false
#define false FALSE
#endif
#ifndef true
#define true TRUE
#endif
#endif
#endif

/*****************************************************************************/

#endif /* MP4V2_PLATFORM_H */
