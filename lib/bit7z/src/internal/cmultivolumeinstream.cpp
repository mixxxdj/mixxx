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

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "internal/cmultivolumeinstream.hpp"
#include "internal/util.hpp"
#include "internal/fsutil.hpp"

namespace bit7z {

CMultiVolumeInStream::CMultiVolumeInStream( const fs::path& firstVolume ) : mCurrentPosition{ 0 }, mTotalSize{ 0 } {
    constexpr size_t kVolumeDigits = 3u;
    size_t volumeIndex = 1u;
    fs::path volumePath = firstVolume;
    while ( fs::exists( volumePath ) ) {
        addVolume( volumePath );

        ++volumeIndex;
        tstring volumeExt = to_tstring( volumeIndex );
        if ( volumeExt.length() < kVolumeDigits ) {
            volumeExt.insert( volumeExt.begin(), kVolumeDigits - volumeExt.length(), BIT7Z_STRING( '0' ) );
        }
        volumePath.replace_extension( volumeExt );

        // TODO: Avoid keeping all the volumes streams open
        constexpr auto kOpenedFilesThreshold = 500;
        if ( volumeIndex == kOpenedFilesThreshold ) {
            // Note: we use == to avoid increasing the limit more than once;
            // the volumeIndex is always increasing, so it is not an issue here.
            filesystem::fsutil::increase_opened_files_limit();
        }
    }
}

auto CMultiVolumeInStream::currentVolume() -> const CMyComPtr< CVolumeInStream >& {
    size_t left = 0;
    size_t right = mVolumes.size();
    size_t midpoint = right / 2;
    while ( true ) {
        auto& volume = mVolumes[ midpoint ];
        if ( mCurrentPosition < volume->globalOffset() ) {
            right = midpoint;
        } else if ( mCurrentPosition >= volume->globalOffset() + volume->size() ) {
            left = midpoint + 1;
        } else {
            return volume;
        }
        midpoint = ( left + right ) / 2;
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeInStream::Read( void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 || mCurrentPosition >= mTotalSize ) {
        return S_OK;
    }

    const auto& volume = currentVolume();
    UInt64 localOffset = mCurrentPosition - volume->globalOffset();
    HRESULT result = volume->Seek( static_cast< Int64 >( localOffset ), STREAM_SEEK_SET, &localOffset );
    if ( result != S_OK ) {
        return result;
    }

    const uint64_t remaining = volume->size() - localOffset;
    if ( size > remaining ) {
        size = static_cast< UInt32 >( remaining );
    }
    result = volume->Read( data, size, &size );
    mCurrentPosition += size;

    if ( processedSize != nullptr ) {
        *processedSize = size;
    }
    return result;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    uint64_t seekPosition{};
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            break;
        case STREAM_SEEK_CUR:
            seekPosition = mCurrentPosition;
            break;
        case STREAM_SEEK_END:
            seekPosition = mTotalSize;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    RINOK( seek_to_offset( seekPosition, offset ) )
    mCurrentPosition = seekPosition;

    if ( newPosition != nullptr ) {
        *newPosition = mCurrentPosition;
    }
    return S_OK;
}

void CMultiVolumeInStream::addVolume( const fs::path& volumePath ) {
    uint64_t globalOffset = 0;
    if ( !mVolumes.empty() ) {
        const auto& lastStream = mVolumes.back();
        globalOffset = lastStream->globalOffset() + lastStream->size();
    }
    mVolumes.emplace_back( make_com< CVolumeInStream >( volumePath, globalOffset ) );
    mTotalSize += mVolumes.back()->size();
}

} // namespace bit7z