/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CBUFFERINSTREAM_HPP
#define CBUFFERINSTREAM_HPP

#include "bittypes.hpp"
#include "internal/com.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>

namespace bit7z {

using std::vector;

class CBufferInStream final : public IInStream, public CMyUnknownImp {
    public:
        explicit CBufferInStream( const vector< byte_t >& inBuffer );

        CBufferInStream( const CBufferInStream& ) = delete;

        CBufferInStream( CBufferInStream&& ) = delete;

        auto operator=( const CBufferInStream& ) -> CBufferInStream& = delete;

        auto operator=( CBufferInStream&& ) -> CBufferInStream& = delete;

        MY_UNKNOWN_DESTRUCTOR( ~CBufferInStream() ) = default;

        // IInStream
        BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( IInStream )  //-V2507 //-V2511 //-V835

    private:
        const buffer_t& mBuffer;
        buffer_t::const_iterator mCurrentPosition;
};

}  // namespace bit7z

#endif // CBUFFERINSTREAM_HPP
