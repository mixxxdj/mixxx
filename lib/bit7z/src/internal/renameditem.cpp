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

#include "internal/dateutil.hpp"
#include "internal/fsutil.hpp"
#include "internal/renameditem.hpp"
#include "internal/stringutil.hpp"

namespace bit7z {

RenamedItem::RenamedItem( const BitInputArchive& inputArchive, uint32_t index, const tstring& newPath )
    : mInputArchive{ inputArchive }, mIndex{ index }, mNewPath{ tstring_to_path( newPath ) } {}

auto RenamedItem::name() const -> tstring {
    return path_to_tstring( mNewPath.filename() );
}

auto RenamedItem::path() const -> tstring {
    return path_to_tstring( mNewPath );
}

auto RenamedItem::inArchivePath() const -> fs::path {
    return mNewPath;
}

auto RenamedItem::getStream( ISequentialInStream** /*inStream*/ ) const noexcept -> HRESULT {
    return S_OK;
}

auto RenamedItem::hasNewData() const noexcept -> bool {
    return false; //just a new property (i.e., path/name), no new data!
}

auto RenamedItem::isDir() const -> bool {
    return mInputArchive.itemProperty( mIndex, BitProperty::IsDir ).getBool();
}

auto RenamedItem::isSymLink() const -> bool {
    return mInputArchive.itemAt( mIndex ).isSymLink();
}

auto RenamedItem::size() const -> uint64_t {
    return mInputArchive.itemProperty( mIndex, BitProperty::Size ).getUInt64();
}

auto RenamedItem::creationTime() const -> FILETIME {
    const BitPropVariant creationTime = mInputArchive.itemProperty( mIndex, BitProperty::CTime );
    return creationTime.isFileTime() ? creationTime.getFileTime() : current_file_time();
}

auto RenamedItem::lastAccessTime() const -> FILETIME {
    const BitPropVariant accessTime = mInputArchive.itemProperty( mIndex, BitProperty::ATime );
    return accessTime.isFileTime() ? accessTime.getFileTime() : current_file_time();
}

auto RenamedItem::lastWriteTime() const -> FILETIME {
    const BitPropVariant writeTime = mInputArchive.itemProperty( mIndex, BitProperty::MTime );
    return writeTime.isFileTime() ? writeTime.getFileTime() : current_file_time();
}

auto RenamedItem::attributes() const -> uint32_t {
    return mInputArchive.itemProperty( mIndex, BitProperty::Attrib ).getUInt32();
}

} // namespace bit7z