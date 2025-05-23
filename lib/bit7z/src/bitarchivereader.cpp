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

#include <algorithm>
#include <numeric>

#include "bitarchivereader.hpp"
#include "internal/operationresult.hpp"
#include "internal/stringutil.hpp"

using namespace bit7z;

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    const tstring& inArchive,
                                    ArchiveStartOffset archiveStart,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, inArchive, archiveStart ) {}

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    const tstring& inArchive,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, inArchive ) {}

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    const buffer_t& inArchive,
                                    ArchiveStartOffset archiveStart,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, inArchive, archiveStart ) {}

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    const buffer_t& inArchive,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, inArchive ) {}

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    std::istream& inArchive,
                                    ArchiveStartOffset archiveStart,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, inArchive, archiveStart ) {}

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    std::istream& inArchive,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, inArchive ) {}

auto BitArchiveReader::archiveProperties() const -> map< BitProperty, BitPropVariant > {
    map< BitProperty, BitPropVariant > result;
    for ( uint32_t i = kpidNoProperty; i <= kpidCopyLink; ++i ) {
        // We cast property twice (here and in archiveProperty), to make the code is easier to read.
        const auto property = static_cast< BitProperty >( i );
        const BitPropVariant propertyValue = archiveProperty( property );
        if ( !propertyValue.isEmpty() ) {
            result[ property ] = propertyValue;
        }
    }
    return result;
}

auto BitArchiveReader::items() const -> std::vector< BitArchiveItemInfo > {
    const auto count = itemsCount();

    std::vector< BitArchiveItemInfo > result;
    result.reserve( count );
    for ( uint32_t i = 0; i < count; ++i ) {
        BitArchiveItemInfo item( i );
        for ( uint32_t j = kpidNoProperty; j <= kpidCopyLink; ++j ) {
            // We cast property twice (here and in archiveProperty), to make the code is easier to read.
            const auto property = static_cast< BitProperty >( j );
            const auto propertyValue = itemProperty( i, property );
            if ( !propertyValue.isEmpty() ) {
                item.setProperty( property, propertyValue );
            }
        }
        result.emplace_back( std::move( item ) );
    }
    return result;
}

auto BitArchiveReader::foldersCount() const -> uint32_t {
    return std::count_if( cbegin(), cend(), []( const BitArchiveItem& item ) {
        return item.isDir();
    } );
}

auto BitArchiveReader::filesCount() const -> uint32_t {
    return itemsCount() - foldersCount(); // I'm lazy :)
}

auto BitArchiveReader::size() const -> uint64_t {
    return std::accumulate( cbegin(), cend(), 0ull, []( uint64_t accumulator, const BitArchiveItem& item ) {
        return item.isDir() ? accumulator : accumulator + item.size();
    } );
}

auto BitArchiveReader::packSize() const -> uint64_t {
    return std::accumulate( cbegin(), cend(), 0ull, []( uint64_t accumulator, const BitArchiveItem& item ) {
        return item.isDir() ? accumulator : accumulator + item.packSize();
    } );
}

auto BitArchiveReader::hasEncryptedItems() const -> bool {
    /* Note: simple encryption (i.e., not including the archive headers) can be detected only reading
     *       the properties of the files in the archive, so we search for any encrypted file inside the archive. */
    return std::any_of( cbegin(), cend(), []( const BitArchiveItem& item ) {
        return !item.isDir() && item.isEncrypted();
    } );
}

auto BitArchiveReader::isEncrypted() const -> bool {
    if ( filesCount() == 0 ) {
        return false;
    }
    return std::all_of( cbegin(), cend(), []( const BitArchiveItem& item ) {
        return item.isDir() || item.isEncrypted();
    } );
}

auto BitArchiveReader::isMultiVolume() const -> bool {
    if ( extractionFormat() == BitFormat::Split || ends_with( archivePath(), BIT7Z_STRING( ".001" ) ) ) {
        return true;
    }
    const BitPropVariant isMultiVolume = archiveProperty( BitProperty::IsVolume );
    return isMultiVolume.isBool() && isMultiVolume.getBool();
}

auto BitArchiveReader::isSolid() const -> bool {
    const BitPropVariant isSolid = archiveProperty( BitProperty::Solid );
    return isSolid.isBool() && isSolid.getBool();
}

auto BitArchiveReader::volumesCount() const -> std::uint32_t {
    if ( extractionFormat() != BitFormat::Split && ends_with( archivePath(), BIT7Z_STRING( ".001" ) ) ) {
        constexpr size_t kVolumeDigits = 3u;

        std::uint32_t result = 2u;
        fs::path volumePath = tstring_to_path( archivePath() );
        do {
            tstring volumeExt = to_tstring( result++ );
            if ( volumeExt.length() < kVolumeDigits ) {
                volumeExt.insert( volumeExt.begin(), kVolumeDigits - volumeExt.length(), BIT7Z_STRING( '0' ) );
            }
            volumePath.replace_extension( volumeExt );
        } while ( fs::exists( volumePath ) );
        return result - 2;
    }

    const BitPropVariant volumesCount = archiveProperty( BitProperty::NumVolumes );
    return volumesCount.isEmpty() ? 1 : volumesCount.getUInt32();
}

auto BitArchiveReader::isOpenEncryptedError( std::error_code error ) -> bool {
    static const auto encryptedError = make_error_code( OperationResult::OpenErrorEncrypted );
    return error == encryptedError;
}
