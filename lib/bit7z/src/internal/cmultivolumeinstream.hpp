/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CMULTIVOLUMEINSTREAM_HPP
#define CMULTIVOLUMEINSTREAM_HPP

#include "internal/com.hpp"
#include "internal/cvolumeinstream.hpp"
#include "internal/macros.hpp"
#include "internal/guiddef.hpp"

#include <7zip/IStream.h>

namespace bit7z {

class CMultiVolumeInStream : public IInStream, public CMyUnknownImp {
        uint64_t mCurrentPosition;
        uint64_t mTotalSize;

        std::vector< CMyComPtr< CVolumeInStream > > mVolumes;

        auto currentVolume() -> const CMyComPtr< CVolumeInStream >&;

        void addVolume( const fs::path& volumePath );

    public:
        explicit CMultiVolumeInStream( const fs::path& firstVolume );

        CMultiVolumeInStream( const CMultiVolumeInStream& ) = delete;

        CMultiVolumeInStream( CMultiVolumeInStream&& ) = delete;

        auto operator=( const CMultiVolumeInStream& ) -> CMultiVolumeInStream& = delete;

        auto operator=( CMultiVolumeInStream&& ) -> CMultiVolumeInStream& = delete;

        MY_UNKNOWN_VIRTUAL_DESTRUCTOR( ~CMultiVolumeInStream() ) = default;

        // IInStream
        BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        // NOLINTNEXTLINE(modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( IInStream ) //-V2507 //-V2511 //-V835
};

} // namespace bit7z

#endif //CMULTIVOLUMEINSTREAM_HPP
