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

#include "internal/cvolumeoutstream.hpp"

namespace bit7z {

CVolumeOutStream::CVolumeOutStream( const fs::path& volumeName )
    : CFileOutStream( volumeName ), mCurrentOffset{ 0 }, mCurrentSize{ 0 } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CVolumeOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    UInt64 pos{};
    RINOK( CStdOutStream::Seek( offset, seekOrigin, &pos ) )
    mCurrentOffset = pos;
    if ( newPosition != nullptr ) {
        *newPosition = pos;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CVolumeOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    UInt32 writtenSize{};
    RINOK( CStdOutStream::Write( data, size, &writtenSize ) )

    if ( writtenSize == 0 && size != 0 ) {
        return E_FAIL;
    }

    mCurrentOffset += writtenSize;
    if ( processedSize != nullptr ) {
        *processedSize += writtenSize;
    }
    return S_OK;
}

auto CVolumeOutStream::currentOffset() const -> uint64_t {
    return mCurrentOffset;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CVolumeOutStream::SetSize( UInt64 newSize ) noexcept {
    RINOK( CFileOutStream::SetSize( newSize ) )
    mCurrentSize = newSize;
    return S_OK;
}

auto CVolumeOutStream::currentSize() const -> uint64_t {
    return mCurrentSize;
}

void CVolumeOutStream::setCurrentSize( uint64_t currentSize ) {
    mCurrentSize = currentSize;
}

} // namespace bit7z