/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CVOLUMEOUTSTREAM_HPP
#define CVOLUMEOUTSTREAM_HPP

#include "internal/cfileoutstream.hpp"

namespace bit7z {

class CVolumeOutStream final : public CFileOutStream {
    public:
        explicit CVolumeOutStream( const fs::path& volumeName );

        BIT7Z_NODISCARD auto currentOffset() const -> uint64_t;

        BIT7Z_NODISCARD auto currentSize() const -> uint64_t;

        void setCurrentSize( uint64_t currentSize );

        // IOutStream
        BIT7Z_STDMETHOD( Write, void const* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        BIT7Z_STDMETHOD( SetSize, UInt64 newSize );

    private:
        uint64_t mCurrentOffset;

        uint64_t mCurrentSize;
};

}  // namespace bit7z

#endif //CVOLUMEOUTSTREAM_HPP
