/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CSYMLINKINSTREAM_HPP
#define CSYMLINKINSTREAM_HPP

#include <sstream>

#include "bitdefines.hpp"
#include "internal/cstdinstream.hpp"
#include "internal/fs.hpp"

namespace bit7z {

struct CSymlinkInStream : public IInStream, public CMyUnknownImp {
public:
    explicit CSymlinkInStream( const fs::path& symlinkPath );

    CSymlinkInStream( const CSymlinkInStream& ) = delete;

    CSymlinkInStream( CSymlinkInStream&& ) = delete;

    auto operator=( const CSymlinkInStream& ) -> CSymlinkInStream& = delete;

    auto operator=( CSymlinkInStream&& ) -> CSymlinkInStream& = delete;

    MY_UNKNOWN_VIRTUAL_DESTRUCTOR ( ~CSymlinkInStream() ) = default;

    // IInStream
    BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

    BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

    MY_UNKNOWN_IMP1( IInStream ) //-V2507 //-V2511 //-V835

private:
    std::istringstream mStream;
    CMyComPtr< CStdInStream > mSymlinkStream;
};

} // namespace bit7z

#endif //CSYMLINKINSTREAM_HPP
