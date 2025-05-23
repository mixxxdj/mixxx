/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CFIXEDBUFFEROUTSTREAM_HPP
#define CFIXEDBUFFEROUTSTREAM_HPP

#include "bittypes.hpp"
#include "internal/com.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>

namespace bit7z {

class CFixedBufferOutStream final : public IOutStream, public CMyUnknownImp {
    public:
        explicit CFixedBufferOutStream( byte_t* buffer, std::size_t size );

        CFixedBufferOutStream( const CFixedBufferOutStream& ) = delete;

        CFixedBufferOutStream( CFixedBufferOutStream&& ) = delete;

        auto operator=( const CFixedBufferOutStream& ) -> CFixedBufferOutStream& = delete;

        auto operator=( CFixedBufferOutStream&& ) -> CFixedBufferOutStream& = delete;

        MY_UNKNOWN_DESTRUCTOR( ~CFixedBufferOutStream() ) = default;

        // IOutStream
        BIT7Z_STDMETHOD( Write, const void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        BIT7Z_STDMETHOD( SetSize, UInt64 newSize );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( IOutStream ) //-V2507 //-V2511 //-V835

    private:
        byte_t* mBuffer;
        size_t mBufferSize;
        size_t mCurrentPosition;
};

}  // namespace bit7z

#endif // CFIXEDBUFFEROUTSTREAM_HPP
