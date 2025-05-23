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

#include <algorithm> //for std::copy_n

#include "internal/cbufferoutstream.hpp"
#include "internal/bufferutil.hpp"

namespace bit7z {

CBufferOutStream::CBufferOutStream( vector< byte_t >& outBuffer )
    : mBuffer( outBuffer ), mCurrentPosition{ mBuffer.begin() } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferOutStream::SetSize( UInt64 newSize ) noexcept {
    try {
        mBuffer.resize( static_cast< vector< byte_t >::size_type >( newSize ) );
        return S_OK;
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    uint64_t newIndex{};
    const HRESULT res = seek( mBuffer, mCurrentPosition, offset, seekOrigin, newIndex );

    if ( res != S_OK ) {
        // We failed to seek (e.g., the new index would not be in the range [0, mBuffer.size]).
        return res;
    }

    // Note: newIndex can be equal to mBuffer.size(); in this case, mCurrentPosition == mBuffer.cend()
    mCurrentPosition = mBuffer.begin() + static_cast< index_t >( newIndex );

    if ( newPosition != nullptr ) {
        *newPosition = newIndex;
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CBufferOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( data == nullptr || size == 0 ) {
        return E_FAIL;
    }

    auto oldPos = ( mCurrentPosition - mBuffer.begin() );
    const size_t newPos = static_cast< size_t >( oldPos ) + static_cast< size_t >( size );
    if ( newPos > mBuffer.size() ) {
        try {
            mBuffer.resize( newPos );
        } catch ( ... ) {
            return E_OUTOFMEMORY;
        }
        mCurrentPosition = mBuffer.begin() + oldPos; // resize(...) invalidated the old mCurrentPosition iterator
    }

    const auto* byteData = static_cast< const byte_t* >( data ); //-V2571
    try {
        std::copy_n( byteData, size, mCurrentPosition );
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }

    std::advance( mCurrentPosition, clamp_cast< std::ptrdiff_t >( size ) );

    if ( processedSize != nullptr ) {
        *processedSize = size;
    }

    return S_OK;
}

} // namespace bit7z