/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WINDOWS_HPP
#define WINDOWS_HPP

#include "bitwindows.hpp"

#ifndef _WIN32

#include <Common/MyTypes.h> // For HRESULT_FROM_WIN32.

/* Note: we must avoid including other headers of p7zip, like stdafx.h! */

/* Making sure constants and type aliases declared in bitwindows.hpp are usable by p7zip
 * as if they were not inside the bit7z namespace. */
using namespace bit7z;

/* Extra Win32 type aliases used by p7zip */
using LPCOLESTR = const OLECHAR*;
using LPCSTR = const char*;
using UINT = unsigned int;
using LONG = int;
using UInt16 = uint16_t;
using UInt32 = uint32_t;

// Win32 file attributes flags
// Note: on Windows, they are signed integers, while here we declare them as unsigned
// to avoid sign conflicts in our code.
#ifndef FILE_ATTRIBUTE_READONLY
constexpr auto FILE_ATTRIBUTE_READONLY       = 1u;
#endif

#ifndef FILE_ATTRIBUTE_HIDDEN
constexpr auto FILE_ATTRIBUTE_HIDDEN         = 2u;
#endif

#ifndef FILE_ATTRIBUTE_DIRECTORY
constexpr auto FILE_ATTRIBUTE_DIRECTORY      = 16u;
#endif

#ifndef FILE_ATTRIBUTE_ARCHIVE
constexpr auto FILE_ATTRIBUTE_ARCHIVE        = 32u;
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
constexpr auto FILE_ATTRIBUTE_NORMAL         = 128u;
#endif

#ifndef FILE_ATTRIBUTE_REPARSE_POINT
constexpr auto FILE_ATTRIBUTE_REPARSE_POINT  = 1024u;
#endif

constexpr auto MAX_PATHNAME_LEN = 1024;

// Win32 VARIANT_BOOL constants
constexpr auto VARIANT_TRUE = static_cast< VARIANT_BOOL >( -1 );
constexpr auto VARIANT_FALSE = static_cast< VARIANT_BOOL >( 0 );

// Win32 macros needed by p7zip code
#ifndef FAILED
#define FAILED( Status ) (static_cast< HRESULT >(Status)<0)
#endif

#ifndef HRESULT_FACILITY
#define HRESULT_FACILITY( hr )  (((hr) >> 16) & 0x1FFF)
#endif

#ifndef HRESULT_CODE
#define HRESULT_CODE( hr )    ((hr) & 0xFFFF)
#endif

// Win32 APIs
inline auto WINAPI GetLastError() -> DWORD { return static_cast< DWORD >( errno ); }

constexpr auto FACILITY_ERRNO = 0x800;
constexpr auto FACILITY_WIN32 = 7;

#ifndef HRESULT_FROM_WIN32 // for p7zip (7-zip declares HRESULT_FROM_WIN32 in C/7zTypes.h so there's no need for this).
constexpr auto FACILITY_CODE = FACILITY_WIN32;
constexpr auto WIN32_MASK = 0x0000FFFF;

/* Note: p7zip uses FACILITY_WIN32, 7-zip version of HRESULT_FROM_WIN32 uses FACILITY_ERRNO. */
inline constexpr auto HRESULT_FROM_WIN32( unsigned int x ) -> HRESULT {
    auto res = static_cast< HRESULT >( x );
    return ( res > 0 ) ? static_cast< HRESULT >( ( x & WIN32_MASK ) | ( FACILITY_WIN32 << 16u ) | 0x80000000 ) : res;
}

constexpr auto ERROR_NEGATIVE_SEEK = 0x100131;
#endif

// Win32 structs
struct WIN32_FILE_ATTRIBUTE_DATA {
    uint32_t dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
};

// Win32 enums
enum VARENUM {
    VT_EMPTY = 0,
    VT_NULL = 1,
    VT_I2 = 2,
    VT_I4 = 3,
    VT_R4 = 4,
    VT_R8 = 5,
    VT_CY = 6,
    VT_DATE = 7,
    VT_BSTR = 8,
    VT_DISPATCH = 9,
    VT_ERROR = 10,
    VT_BOOL = 11,
    VT_VARIANT = 12,
    VT_UNKNOWN = 13,
    VT_DECIMAL = 14,
    VT_I1 = 16,
    VT_UI1 = 17,
    VT_UI2 = 18,
    VT_UI4 = 19,
    VT_I8 = 20,
    VT_UI8 = 21,
    VT_INT = 22,
    VT_UINT = 23,
    VT_VOID = 24,
    VT_HRESULT = 25,
    VT_FILETIME = 64
};

enum STREAM_SEEK {
    STREAM_SEEK_SET = 0,
    STREAM_SEEK_CUR = 1,
    STREAM_SEEK_END = 2
};

#ifndef MY_EXTERN_C
#define MY_EXTERN_C extern "C"
#endif

#ifndef EXTERN_C // 7-zip 23.01+
#define EXTERN_C MY_EXTERN_C
#endif

// String-related Win32 API functions (implemented in windows.cpp)
auto SysAllocStringByteLen( LPCSTR psz, UINT len ) -> BSTR;

auto SysAllocStringLen( const OLECHAR*, UINT ) -> BSTR;

auto SysAllocString( const OLECHAR* str ) -> BSTR;

void SysFreeString( BSTR bstr );

auto SysStringByteLen( BSTR bstr ) -> UINT;

auto SysStringLen( BSTR bstr ) -> UINT;

#endif

#ifndef __HRESULT_FROM_WIN32
#define __HRESULT_FROM_WIN32 HRESULT_FROM_WIN32 // NOLINT(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
#endif

/* For when we cannot include IStream.h */
#ifndef HRESULT_WIN32_ERROR_NEGATIVE_SEEK
#ifdef MY__E_ERROR_NEGATIVE_SEEK // 7-zip on Unix
// On Unix 7-zip uses a different facility code since it uses HRESULT_FROM_WIN32 only for POSIX error codes.
constexpr auto FACILITY_CODE = FACILITY_ERRNO;
constexpr auto HRESULT_WIN32_ERROR_NEGATIVE_SEEK = MY__E_ERROR_NEGATIVE_SEEK;
#elif defined( MY_E_ERROR_NEGATIVE_SEEK ) // 7-zip 23.01+ on Unix
constexpr auto FACILITY_CODE = FACILITY_ERRNO;
constexpr auto HRESULT_WIN32_ERROR_NEGATIVE_SEEK = MY_E_ERROR_NEGATIVE_SEEK;
#else // p7zip or 7-zip on Windows
#ifdef _WIN32 // 7-zip on Windows
constexpr auto FACILITY_CODE = FACILITY_WIN32;
#endif
constexpr auto HRESULT_WIN32_ERROR_NEGATIVE_SEEK = __HRESULT_FROM_WIN32( ERROR_NEGATIVE_SEEK );
#endif
#endif

// Note: this needs to be defined on all platforms, as it is a custom file attributes extension defined by p7zip/7-zip.
#ifndef FILE_ATTRIBUTE_UNIX_EXTENSION
constexpr auto FILE_ATTRIBUTE_UNIX_EXTENSION = 0x8000; // As defined by p7zip
#endif

#endif //WINDOWS_HPP
