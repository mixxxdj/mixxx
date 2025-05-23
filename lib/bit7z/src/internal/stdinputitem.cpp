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

#include <utility>

#include "internal/cstdinstream.hpp"
#include "internal/dateutil.hpp"
#include "internal/stdinputitem.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"

using std::istream;

namespace bit7z {

StdInputItem::StdInputItem( istream& stream, fs::path path ) : mStream{ stream }, mStreamPath{ std::move( path ) } {}

auto StdInputItem::name() const -> tstring {
    return path_to_tstring( mStreamPath.filename() );
}

auto StdInputItem::path() const -> tstring {
    return path_to_tstring( mStreamPath );
}

auto StdInputItem::inArchivePath() const -> fs::path {
    return mStreamPath;
}

auto StdInputItem::getStream( ISequentialInStream** inStream ) const -> HRESULT {
    auto inStreamLoc = bit7z::make_com< CStdInStream, ISequentialInStream >( mStream );
    *inStream = inStreamLoc.Detach(); //Note: 7-zip will take care of freeing the memory!
    return S_OK;
}

auto StdInputItem::isDir() const noexcept -> bool {
    return false;
}

auto StdInputItem::size() const -> uint64_t {
    const auto originalPos = mStream.tellg();
    mStream.seekg( 0, std::ios::end ); // seeking to the end of the stream
    const auto result = static_cast< uint64_t >( mStream.tellg() - originalPos ); // size of the stream
    mStream.seekg( originalPos ); // seeking back to the original position in the stream
    return result;
}

auto StdInputItem::creationTime() const noexcept -> FILETIME { //-V524
    return current_file_time();
}

auto StdInputItem::lastAccessTime() const noexcept -> FILETIME { //-V524
    return current_file_time();
}

auto StdInputItem::lastWriteTime() const noexcept -> FILETIME {
    return current_file_time();
}

auto StdInputItem::attributes() const noexcept -> uint32_t {
    return static_cast< uint32_t >( FILE_ATTRIBUTE_NORMAL );
}

} // namespace bit7z