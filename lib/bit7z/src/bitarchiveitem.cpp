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

#include "bitarchiveitem.hpp"
#include "internal/fsutil.hpp"
#include "internal/stringutil.hpp"

// For checking posix file attributes
#include <sys/stat.h>

using namespace bit7z;
using namespace bit7z::filesystem;

BitArchiveItem::BitArchiveItem( uint32_t itemIndex ) noexcept
    : mItemIndex( itemIndex ) {}

auto BitArchiveItem::index() const noexcept -> uint32_t {
    return mItemIndex;
}

auto BitArchiveItem::isDir() const -> bool {
    const BitPropVariant isDir = itemProperty( BitProperty::IsDir );
    return !isDir.isEmpty() && isDir.getBool();
}

BIT7Z_NODISCARD
inline auto filename( const fs::path& path ) -> tstring {
    return path_to_tstring( path.filename() );
}

auto BitArchiveItem::name() const -> tstring {
    BitPropVariant name = itemProperty( BitProperty::Name );
    if ( name.isEmpty() ) {
        name = itemProperty( BitProperty::Path );
        return name.isEmpty() ? tstring{} : filename( name.getNativeString() );
    }
    return name.getString();
}

auto BitArchiveItem::extension() const -> tstring {
    if ( isDir() ) {
        return tstring{};
    }
    const BitPropVariant extension = itemProperty( BitProperty::Extension );
    return extension.isEmpty() ? fsutil::extension( name() ) : extension.getString();
}

auto BitArchiveItem::path() const -> tstring {
    BitPropVariant path = itemProperty( BitProperty::Path );
    if ( path.isEmpty() ) {
        path = itemProperty( BitProperty::Name );
        return path.isEmpty() ? tstring{} : path.getString();
    }
    return path.getString();
}

auto BitArchiveItem::nativePath() const -> native_string {
    BitPropVariant path = itemProperty( BitProperty::Path );
    if ( path.isEmpty() ) {
        path = itemProperty( BitProperty::Name );
        return path.isEmpty() ? native_string{} : path.getNativeString();
    }
    return path.getNativeString();
}

auto BitArchiveItem::size() const -> uint64_t {
    const BitPropVariant size = itemProperty( BitProperty::Size );
    return size.isEmpty() ? 0 : size.getUInt64();
}

auto BitArchiveItem::packSize() const -> uint64_t {
    const BitPropVariant packSize = itemProperty( BitProperty::PackSize );
    return packSize.isEmpty() ? 0 : packSize.getUInt64();
}

auto BitArchiveItem::isEncrypted() const -> bool {
    const BitPropVariant isEncrypted = itemProperty( BitProperty::Encrypted );
    return isEncrypted.isBool() && isEncrypted.getBool();
}

auto BitArchiveItem::creationTime() const -> time_type {
    const BitPropVariant creationTime = itemProperty( BitProperty::CTime );
    return creationTime.isFileTime() ? creationTime.getTimePoint() : time_type::clock::now();
}

auto BitArchiveItem::lastAccessTime() const -> time_type {
    const BitPropVariant accessTime = itemProperty( BitProperty::ATime );
    return accessTime.isFileTime() ? accessTime.getTimePoint() : time_type::clock::now();
}

auto BitArchiveItem::lastWriteTime() const -> time_type {
    const BitPropVariant writeTime = itemProperty( BitProperty::MTime );
    return writeTime.isFileTime() ? writeTime.getTimePoint() : time_type::clock::now();
}

auto BitArchiveItem::attributes() const -> uint32_t {
    const BitPropVariant attrib = itemProperty( BitProperty::Attrib );
    return attrib.isUInt32() ? attrib.getUInt32() : 0;
}

auto BitArchiveItem::crc() const -> uint32_t {
    const BitPropVariant crc = itemProperty( BitProperty::CRC );
    return crc.isUInt32() ? crc.getUInt32() : 0;
}

// On MSVC, these macros are not defined!
#if !defined(S_ISLNK) && defined(S_IFMT)
#ifndef S_IFLNK
constexpr auto S_IFLNK = 0xA000;
#endif
#define S_ISLNK( m ) (((m) & S_IFMT) == S_IFLNK)
#endif

auto BitArchiveItem::isSymLink() const -> bool {
    const BitPropVariant symlink = itemProperty( BitProperty::SymLink );
    if ( symlink.isString() ) {
        return true;
    }

    const auto itemAttributes = attributes();
    if ( ( itemAttributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) == FILE_ATTRIBUTE_UNIX_EXTENSION ) {
        auto posixAttributes = itemAttributes >> 16U;
        return S_ISLNK( posixAttributes );
    }
    return ( itemAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) == FILE_ATTRIBUTE_REPARSE_POINT;
}
