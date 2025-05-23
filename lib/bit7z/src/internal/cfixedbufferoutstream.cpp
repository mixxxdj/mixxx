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
#define _SCL_SECURE_NO_WARNINGS
#endif

#include <algorithm> // for std::copy_n

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/cfixedbufferoutstream.hpp"
#include "internal/util.hpp"

namespace bit7z {

CFixedBufferOutStream::CFixedBufferOutStream( byte_t* buffer, std::size_t size )
    : mBuffer( buffer ), mBufferSize( size ), mCurrentPosition( 0 ) {
    if ( size == 0 ) {
        throw BitException( "Could not initialize output buffer stream",
                            make_error_code( BitError::InvalidOutputBufferSize ) );
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::SetSize( UInt64 newSize ) noexcept {
    return newSize != mBufferSize ? E_INVALIDARG : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    uint64_t seekIndex{};
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET: {
            break;
        }
        case STREAM_SEEK_CUR: {
            seekIndex = mCurrentPosition;
            break;
        }
        case STREAM_SEEK_END: {
            seekIndex = mBufferSize;
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    RINOK( seek_to_offset( seekIndex, offset ) )

    // Making sure seekIndex is a valid index within the buffer (i.e., it is less than mBufferSize).
    if ( seekIndex >= mBufferSize ) {
        return E_INVALIDARG;
    }

    mCurrentPosition = clamp_cast< size_t >( seekIndex );

    if ( newPosition != nullptr ) {
        *newPosition = seekIndex;
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CFixedBufferOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( data == nullptr || size == 0 ) {
        return E_FAIL;
    }

    auto writeSize = static_cast< size_t >( size );
    size_t remainingSize = mBufferSize - mCurrentPosition; // The Seek method ensures mCurrentPosition < mBufferSize.
    if ( writeSize > remainingSize ) {
        /* Writing only to the remaining part of the output buffer!
         * Note: since size is an uint32_t, and size >= mBufferSize - mCurrentPosition, the cast is safe. */
        writeSize = remainingSize;
    }

    const auto* byteData = static_cast< const byte_t* >( data ); //-V2571
    try {
        // TODO: Use std::span, gsl::span, or a custom span class (e.g., BufferView).
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::copy_n( byteData, writeSize, &mBuffer[ mCurrentPosition ] ); //-V2563
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }

    mCurrentPosition += writeSize;

    if ( processedSize != nullptr ) {
        // Note: writeSize is not greater than size, which is UInt32, so the cast is safe.
        *processedSize = static_cast< UInt32 >( writeSize );
    }

    return S_OK;
}

} // namespace bit7z