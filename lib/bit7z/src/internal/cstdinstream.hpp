/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CSTDINSTREAM_HPP
#define CSTDINSTREAM_HPP

#include <cstdint>
#include <istream>

#include "internal/com.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>

namespace bit7z {

using std::istream;

class CStdInStream : public IInStream, public CMyUnknownImp {
    public:
        explicit CStdInStream( istream& inputStream );

        CStdInStream( const CStdInStream& ) = delete;

        CStdInStream( CStdInStream&& ) = delete;

        auto operator=( const CStdInStream& ) -> CStdInStream& = delete;

        auto operator=( CStdInStream&& ) -> CStdInStream& = delete;

        MY_UNKNOWN_VIRTUAL_DESTRUCTOR( ~CStdInStream() ) = default;

        // IInStream
        BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( IInStream ) //-V2507 //-V2511 //-V835

    private:
        istream& mInputStream;
};

}  // namespace bit7z

#endif // CSTDINSTREAM_HPP
