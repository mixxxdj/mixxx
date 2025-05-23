// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <locale>

#include "internal/stringutil.hpp"

#ifdef _WIN32
#include <windows.h>
#ifdef BIT7Z_USE_SYSTEM_CODEPAGE
#define CODEPAGE CP_ACP
#define CODEPAGE_WC_FLAGS WC_NO_BEST_FIT_CHARS
#else
#define CODEPAGE CP_UTF8
#define CODEPAGE_WC_FLAGS 0
#endif
#else
#ifndef BIT7Z_USE_STANDARD_FILESYSTEM
// GCC 4.9 doesn't have the <codecvt> header; as a workaround,
// we use GHC filesystem's utility functions for string conversions.
#include "internal/fs.hpp"
#else
// The <codecvt> header has been deprecated in C++17; however, there's no real replacement
// (excluding third-party libraries); hence, for now we just disable the deprecation warnings (only here).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <codecvt>
using convert_type = std::codecvt_utf8< wchar_t >;
#endif
#endif

namespace bit7z {

#if !defined( _WIN32 ) || !defined( BIT7Z_USE_NATIVE_STRING )
auto narrow( const wchar_t* wideString, size_t size ) -> std::string {
    if ( wideString == nullptr || size == 0 ) {
        return "";
    }
#ifdef _WIN32
    const int narrowStringSize = WideCharToMultiByte( CODEPAGE,
                                                      CODEPAGE_WC_FLAGS,
                                                      wideString,
                                                      static_cast< int >( size ),
                                                      nullptr,
                                                      0,
                                                      nullptr,
                                                      nullptr );
    if ( narrowStringSize == 0 ) {
        return "";
    }

    std::string result( static_cast< std::string::size_type >( narrowStringSize ), 0 );
    WideCharToMultiByte( CODEPAGE,
                         CODEPAGE_WC_FLAGS,
                         wideString,
                         -1,
                         &result[ 0 ],  // NOLINT(readability-container-data-pointer)
                         static_cast< int >( narrowStringSize ),
                         nullptr,
                         nullptr );
    return result;
#elif !defined( BIT7Z_USE_STANDARD_FILESYSTEM )
    (void)size; // To avoid warnings of unused size argument...
    return fs::detail::toUtf8( wideString );
#else
    std::wstring_convert< convert_type, wchar_t > converter;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return converter.to_bytes( wideString, wideString + size );
#endif
}

auto widen( const std::string& narrowString ) -> std::wstring {
#ifdef _WIN32
    const int narrowStringSize = static_cast< int >( narrowString.size() );
    const int wideStringSize = MultiByteToWideChar( CODEPAGE,
                                                    0,
                                                    narrowString.c_str(),
                                                    narrowStringSize,
                                                    nullptr,
                                                    0 );
    if ( wideStringSize == 0 ) {
        return L"";
    }

    std::wstring result( static_cast< std::wstring::size_type >( wideStringSize ), 0 );
    MultiByteToWideChar( CODEPAGE,
                         0,
                         narrowString.c_str(),
                         narrowStringSize,
                         &result[ 0 ], // NOLINT(readability-container-data-pointer)
                         wideStringSize );
    return result;
#elif !defined( BIT7Z_USE_STANDARD_FILESYSTEM )
    return fs::detail::fromUtf8< std::wstring >( narrowString );
#else
    std::wstring_convert< convert_type, wchar_t > converter;
    return converter.from_bytes( narrowString );
#endif
}
#endif

#if !defined( _WIN32 ) && defined( BIT7Z_USE_STANDARD_FILESYSTEM )
#pragma GCC diagnostic pop
#endif

} // namespace bit7z