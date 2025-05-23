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

#ifndef _WIN32

#include "bittypes.hpp"
#include "internal/util.hpp"
#include "internal/windows.hpp"

#include <iostream>
#include <cstring>
#include <limits>


auto wcsnlen_s( const wchar_t* str, size_t maxSize ) -> size_t {
    if ( str == nullptr || maxSize == 0 ) {
        return 0;
    }

    size_t result = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for ( ; result < maxSize && str[ result ] != L'\0'; ++result ) {
        // continue;
    }
    return result;
}

auto SysAllocString( const OLECHAR* str ) -> BSTR {
    if ( str == nullptr ) {
        return nullptr;
    }

    auto len = static_cast< UINT >( wcsnlen_s( str, FILENAME_MAX ) );
    return SysAllocStringLen( str, len );
}

using bstr_prefix_t = uint32_t;

/* Internal implementation of SysAllocStringByteLen for Unix systems.
 *
 * Notes:
 *   - We use C allocation functions instead of "new" since we must be able to also free BSTR objects
 *     allocated by 7-zip (which uses malloc). Never mix new/delete and malloc/free.
 *   - We use calloc instead of malloc, so that we do not have to manually add the termination character at the end.
 * */
auto AllocStringBuffer( LPCSTR str, uint32_t byteLength ) -> BSTR {
    // Maximum value that can be stored in the BSTR byteLength prefix.
    constexpr auto kMaxPrefixValue = std::numeric_limits< bstr_prefix_t >::max();

    // Max number of bytes that can be stored in the BSTR (excluding the termination char, and the byteLength prefix).
    constexpr auto kMaxLength = kMaxPrefixValue - sizeof( OLECHAR ) - sizeof( bstr_prefix_t );

    if ( byteLength >= kMaxLength ) { // Invalid byteLength parameter
        return nullptr;
    }

    // Length prefix (32 bits, as in Microsoft specs) + string bytes + termination character (32 bits on Linux).
    auto bufferSize = sizeof( bstr_prefix_t ) + byteLength + sizeof( OLECHAR );

    // Allocating memory for storing the BSTR as a byte array.
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
    auto* bstrBuffer = static_cast< bstr_prefix_t* >( std::calloc( bufferSize, sizeof( byte_t ) ) );

    if ( bstrBuffer == nullptr ) { // Failed to allocate memory for the BSTR buffer.
        return nullptr;
    }

    // Storing the number of bytes of the BSTR as a prefix of it.
    *bstrBuffer = byteLength;

    // The actual BSTR must point after the byteLength prefix.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
    BSTR result = reinterpret_cast< BSTR >( bstrBuffer + 1 );
    if ( str != nullptr ) {
        // Copying byte-by-byte the input string to the BSTR.
        // Note: flawfinder warns about not checking for buffer overflows; this is a false alarm,
        // since are using the correct destination size we just allocated using calloc.
        std::memcpy( result, str, byteLength ); // flawfinder: ignore
    }
    return result;
}

auto SysAllocStringLen( const OLECHAR* str, UINT length ) -> BSTR {
    auto byteLength = length * sizeof( OLECHAR );

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return AllocStringBuffer( reinterpret_cast< LPCSTR >( str ), clamp_cast< std::uint32_t >( byteLength ) );
}

auto SysAllocStringByteLen( LPCSTR str, UINT length ) -> BSTR {
    return AllocStringBuffer( str, length );
}

void SysFreeString( BSTR bstrString ) { // NOLINT(readability-non-const-parameter)
    if ( bstrString != nullptr ) {
        // We must delete the original memory buffer, which starts from the BSTR length prefix
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
        auto* bstrBuffer = reinterpret_cast< byte_t* >( bstrString ) - sizeof( bstr_prefix_t );

        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory)
        std::free( static_cast< void* >( bstrBuffer ) );
    }
}

auto SysStringByteLen( BSTR bstrString ) -> UINT { // NOLINT(readability-non-const-parameter)
    if ( bstrString == nullptr ) {
        return 0;
    }
    // If the string is non-null, we return the value stored in the length prefix of the BSTR.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)
    return *( reinterpret_cast< const bstr_prefix_t* >( bstrString ) - 1 );
}

auto SysStringLen( BSTR bstrString ) -> UINT { // NOLINT(readability-non-const-parameter)
    // Same as SysStringByteLen, but we count how many OLECHARs are stored in the BSTR.
    return SysStringByteLen( bstrString ) / static_cast< UINT >( sizeof( OLECHAR ) );
}

#endif