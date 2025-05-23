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

#include "internal/csymlinkinstream.hpp"
#include "internal/util.hpp"

namespace bit7z {

auto read_symlink_as_string( const fs::path& symlinkPath ) noexcept -> std::string {
    std::error_code error;
    return fs::read_symlink( symlinkPath, error ).u8string();
}

CSymlinkInStream::CSymlinkInStream( const fs::path& symlinkPath )
    : mStream{ read_symlink_as_string( symlinkPath ) },
      mSymlinkStream{ bit7z::make_com< CStdInStream >( mStream ) } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CSymlinkInStream::Read( void* data, UInt32 size, UInt32* processedSize ) noexcept {
    return mSymlinkStream->Read( data, size, processedSize );
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CSymlinkInStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    return mSymlinkStream->Seek( offset, seekOrigin, newPosition );
}

} // namespace bit7z