/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef STRINGUTIL_HPP
#define STRINGUTIL_HPP

#include "bittypes.hpp"
#include "internal/fsutil.hpp"

#include <algorithm>
namespace bit7z {

#if defined( BIT7Z_USE_NATIVE_STRING ) && defined( _WIN32 )
// On Windows, with native strings enabled, strings are already wide!
#   define WIDEN( tstr ) tstr
#else
#   define WIDEN( tstr ) bit7z::widen(tstr)

auto narrow( const wchar_t* wideString, size_t size ) -> std::string;

auto widen( const std::string& narrowString ) -> std::wstring;
#endif

inline auto path_to_tstring( const fs::path& path ) -> tstring {
    /* In an ideal world, we should only use fs::path's string< tchar >() function for converting a path to a tstring.
     * However, MSVC converts paths to std::string using the system codepage instead of UTF-8,
     * which is the default encoding of bit7z. */
#if defined( _WIN32 ) && defined( BIT7Z_USE_NATIVE_STRING )
    return path.wstring();
#elif defined( _WIN32 ) && defined( BIT7Z_USE_SYSTEM_CODEPAGE )
    /* If we encounter a path with Unicode characters, MSVC will throw an exception
     * while converting from a fs::path to std::string if any character is invalid in the system codepage.
     * Hence, here we use bit7z's own string conversion function, which substitutes invalid Unicode characters
     * with '?' characters. */
    const auto& native_path = path.native();
    return narrow( native_path.c_str(), native_path.size() );
#else
    return path.u8string();
#endif
}

#if defined( _MSC_VER ) && !defined( BIT7Z_USE_NATIVE_STRING ) && !defined( BIT7Z_USE_SYSTEM_CODEPAGE )
#define PATH_FROM_TSTRING( str ) fs::u8path( str )
#else
#define PATH_FROM_TSTRING( str ) fs::path{ str }
#endif

inline auto tstring_to_path( const tstring& str ) -> fs::path {
#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
    auto result = PATH_FROM_TSTRING( str );
    if ( filesystem::fsutil::should_format_long_path( result ) ) {
        result = filesystem::fsutil::format_long_path( result );
    }
    return result;
#else
    // By default, MSVC treats strings as encoded using the system codepage, but bit7z uses UTF-8.
    return PATH_FROM_TSTRING( str );
#endif
}

inline auto path_to_wide_string( const fs::path& path ) -> std::wstring {
#if defined( _MSC_VER ) || !defined( BIT7Z_USE_STANDARD_FILESYSTEM )
    return path.wstring();
#else
    /* On some compilers and platforms (e.g., GCC before v12.3),
     * the direct conversion of the fs::path to wstring might throw an exception due to unicode characters.
     * So we simply convert to tstring, and then widen it if necessary. */
    return WIDEN( path.string< tchar >() );
#endif
}

inline auto starts_with( const native_string& str, const native_string& prefix ) -> bool {
    return str.rfind( prefix, 0 ) == 0;
}

// Note: the implementation using std::equal seems to be faster than the alternatives in most cases;
// see https://quick-bench.com/q/G9D6M1h11PrwwmqcS7taJoAIdZU for a comparison.
template< typename CharT >
auto ends_with( const std::basic_string< CharT >& str, const std::basic_string< CharT >& suffix ) -> bool {
    return str.size() >= suffix.size() &&
           std::equal( suffix.crbegin(), suffix.crend(), str.crbegin(), str.crbegin() + suffix.size() );
}

template< typename CharT, std::size_t N >
// NOLINTNEXTLINE(*-avoid-c-arrays)
auto ends_with( const std::basic_string< CharT >& str, const CharT (&suffix)[N] ) -> bool {
    // Note: the suffix C array has a null termination character.
    constexpr auto suffixSize = N - 1;
    return str.size() >= suffixSize &&
           std::equal( std::crbegin( suffix ) + 1, std::crend( suffix ), str.crbegin(), str.crbegin() + suffixSize );
}

/**
 * Checks if the given character is a valid path separator on the target platform.
 *
 * @note On Windows, both / and \ are considered as path separators.
 *       On other platforms, only / is considered a path separator.
 *
 * @param character the character value to be checked.
 *
 * @return true if the character is a valid path separator on the target platform, false otherwise.
 */
inline auto isPathSeparator( char character ) -> bool {
#ifdef _WIN32
    return character == '/' || character == '\\';
#else
    return character == '/';
#endif
}

#ifdef _WIN32
inline auto isPathSeparator( wchar_t character ) -> bool {
    return character == L'/' || character == L'\\';
}
#endif

} // namespace bit7z

#endif //STRINGUTIL_HPP
