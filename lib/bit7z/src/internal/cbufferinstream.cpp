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
#ifdef _MSC_VER
// Disable warning
//    C4996: '...': Function call with parameters that may be unsafe
// This is due to the call to std::copy_n with a raw buffer pointer as destination.
#pragma warning(disable:4996)
#endif

#include <algorithm> //for std::copy_n

#include "internal/cbufferinstream.hpp"
#include "internal/bufferutil.hpp"

namespace bit7z {

CBufferInStream::CBufferInStream( const vector< byte_t >& inBuffer )
    : mBuffer( inBuffer ), mCurrentPosition{ mBuffer.begin() } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferInStream::Read( void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 || mCurrentPosition == mBuffer.cend() ) {
        return S_OK;
    }

    /* Note: thanks to CBufferInStream::Seek, we can safely assume mCurrentPosition to always be a valid iterator;
     * so "remaining" will always be > 0 (and casts to unsigned types are safe) */
    std::ptrdiff_t remaining = mBuffer.cend() - mCurrentPosition;
    if ( cmp_greater( remaining, size ) ) {
        /* The remaining buffer still to read is bigger than the read size requested by the user,
         * so we need to read just a "size" number of bytes. */
        remaining = static_cast< std::ptrdiff_t >( size );
    }
    /* Else, the user requested to read a number of bytes greater than or equal to the number
     * of remaining bytes to be read from the buffer.
     * So we just read all the remaining bytes, not more or less. */

    /* Note: here remaining is > 0 */
    std::copy_n( mCurrentPosition, remaining, static_cast< byte_t* >( data ) ); //-V2571
    std::advance( mCurrentPosition, remaining );

    if ( processedSize != nullptr ) {
        /* Note: even though on 64-bit systems "remaining" will be a 64-bit unsigned integer (size_t),
         * its value cannot be greater than "size", which is a 32-bit unsigned int; hence, this cast is safe. */
        *processedSize = static_cast< UInt32 >( remaining );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    uint64_t newIndex{};
    const HRESULT res = seek( mBuffer, mCurrentPosition, offset, seekOrigin, newIndex );

    if ( res != S_OK ) {
        // The newIndex is not in the range [0, mBuffer.size]
        return res;
    }

    // Note: newIndex can be equal to mBuffer.size(); in this case, mCurrentPosition == mBuffer.cend()
    mCurrentPosition = mBuffer.cbegin() + static_cast< index_t >( newIndex );

    if ( newPosition != nullptr ) {
        // Safe cast, since newIndex >= 0
        *newPosition = newIndex;
    }

    return S_OK;
}

} // namespace bit7z