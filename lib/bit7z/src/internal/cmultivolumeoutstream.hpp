/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CMULTIVOLUMEOUTSTREAM_HPP
#define CMULTIVOLUMEOUTSTREAM_HPP

#include <vector>
#include <string>
#include <cstdint>

#include "internal/com.hpp"
#include "internal/guiddef.hpp"
#include "internal/cvolumeoutstream.hpp"

#include <7zip/IStream.h>


using std::vector;
using std::wstring;

namespace bit7z {

class CMultiVolumeOutStream final : public IOutStream, public CMyUnknownImp {
        // Size of a single volume.
        uint64_t mMaxVolumeSize;

        // Common name prefix of every volume.
        fs::path mVolumePrefix;

        // The current volume stream on which we are working.
        size_t mCurrentVolumeIndex;

        // Offset from the beginning of the current volume stream (i.e., the one at mCurrentVolumeIndex).
        uint64_t mCurrentVolumeOffset;

        // Offset from the beginning of the whole output archive.
        uint64_t mAbsoluteOffset;

        // Total size of the output archive (sum of the volumes' sizes).
        uint64_t mFullSize;

        vector< CMyComPtr< CVolumeOutStream > > mVolumes;

    public:
        CMultiVolumeOutStream( uint64_t volSize, fs::path archiveName );

        CMultiVolumeOutStream( const CMultiVolumeOutStream& ) = delete;

        CMultiVolumeOutStream( CMultiVolumeOutStream&& ) = delete;

        auto operator=( const CMultiVolumeOutStream& ) -> CMultiVolumeOutStream& = delete;

        auto operator=( CMultiVolumeOutStream&& ) -> CMultiVolumeOutStream& = delete;

        MY_UNKNOWN_DESTRUCTOR( ~CMultiVolumeOutStream() ) = default;

        // IOutStream
        BIT7Z_STDMETHOD( Write, const void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        BIT7Z_STDMETHOD( SetSize, UInt64 newSize );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( IOutStream ) //-V2507 //-V2511 //-V835
};

}  // namespace bit7z

#endif // CMULTIVOLUMEOUTSTREAM_HPP
