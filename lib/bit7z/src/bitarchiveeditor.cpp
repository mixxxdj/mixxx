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

#include "bitarchiveeditor.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/bufferitem.hpp"
#include "internal/fsitem.hpp"
#include "internal/renameditem.hpp"
#include "internal/stdinputitem.hpp"
#include "internal/stringutil.hpp"

namespace bit7z {

using std::istream;

BitArchiveEditor::BitArchiveEditor( const Bit7zLibrary& lib,
                                    const tstring& inFile,
                                    const BitInOutFormat& format,
                                    const tstring& password )
    : BitArchiveWriter( lib, inFile, ArchiveStartOffset::FileStart, format, password ) {
    if ( inputArchive() != nullptr ) {
        return; // Input file was correctly read by base class BitOutputArchive constructor
    }

    /* Note: BitArchiveWriter doesn't require an input file, but BitArchiveEditor does! */
    if ( inFile.empty() ) {
        throw BitException( "Could not open archive", make_error_code( BitError::InvalidArchivePath ) );
    }

    /* Note: if we are here, a non-empty inFile was specified, but the BitOutputArchive constructor
     *       left a nullptr mInputArchive.
     *       This means that inFile doesn't exist (see BitOutputArchive's constructor).
     *       There's no need to check again for its existence (e.g., using fs::exists). */
    throw BitException( "Could not open archive",
                        std::make_error_code( std::errc::no_such_file_or_directory ),
                        inFile );
}

BitArchiveEditor::~BitArchiveEditor() = default;

void BitArchiveEditor::renameItem( uint32_t index, const tstring& newPath ) {
    checkIndex( index );
    mEditedItems[ index ] = std::make_unique< RenamedItem >( *inputArchive(), index, newPath ); //-V108
}

void BitArchiveEditor::renameItem( const tstring& oldPath, const tstring& newPath ) {
    auto index = findItem( oldPath );
    mEditedItems[ index ] = std::make_unique< RenamedItem >( *inputArchive(), index, newPath ); //-V108
}

void BitArchiveEditor::updateItem( uint32_t index, const tstring& inFile ) {
    checkIndex( index );
    auto itemName = inputArchive()->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< FilesystemItem >( tstring_to_path( inFile ), itemName.getNativeString() ); //-V108
}

void BitArchiveEditor::updateItem( uint32_t index, const std::vector< byte_t >& inBuffer ) {
    checkIndex( index );
    auto itemName = inputArchive()->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< BufferItem >( inBuffer, itemName.getNativeString() ); //-V108
}

void BitArchiveEditor::updateItem( uint32_t index, std::istream& inStream ) {
    checkIndex( index );
    auto itemName = inputArchive()->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< StdInputItem >( inStream, itemName.getNativeString() ); //-V108
}

void BitArchiveEditor::updateItem( const tstring& itemPath, const tstring& inFile ) {
    mEditedItems[ findItem( itemPath ) ] = std::make_unique< FilesystemItem >( tstring_to_path( inFile ), //-V108
                                                                               tstring_to_path( itemPath ) );
}

void BitArchiveEditor::updateItem( const tstring& itemPath, const std::vector< byte_t >& inBuffer ) {
    mEditedItems[ findItem( itemPath ) ] = std::make_unique< BufferItem >( inBuffer, itemPath ); //-V108
}

void BitArchiveEditor::updateItem( const tstring& itemPath, std::istream& inStream ) {
    mEditedItems[ findItem( itemPath ) ] = std::make_unique< StdInputItem >( inStream, itemPath ); //-V108
}

void BitArchiveEditor::deleteItem( uint32_t index, DeletePolicy policy ) {
    if ( index >= inputArchiveItemsCount() ) {
        throw BitException( "Cannot delete item at index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    markItemAsDeleted( index );

    const auto& deletedItem = inputArchive()->itemAt( index );
    if ( !deletedItem.isDir() || policy == DeletePolicy::ItemOnly ) {
        return;
    }

    const auto deletedPath = deletedItem.nativePath() + fs::path::preferred_separator;
    if ( deletedPath.size() <= 1 ) { // The original path was empty
        return;
    }

    for ( const auto& item: *inputArchive() ) {
        if ( starts_with( item.nativePath(), deletedPath ) ) {
            markItemAsDeleted( item.index() );
        }
    }
}

/**
 * Determines if the given itemPath is within the specified base directory.
 *
 * @param itemPath The path to the item to be checked.
 * @param isDir    A boolean indicating whether the itemPath represents a directory.
 * @param base     The base directory against which the itemPath is checked.
 *
 * @return true if the itemPath is located within the base directory, false otherwise.
 */
inline auto isInsidePath( const fs::path& itemPath, bool isDir, const fs::path& base ) -> bool {
    const auto relativePath = itemPath.lexically_relative( base ).native();
    /* The relative path of the item with respect to the base directory can be:
     * 1) A single dot component;
     * 2) A path not starting with a dot-dot component (i.e., the item is inside the base path);
     * 3) A path starting with a dot-dot component (i.e., the item is outside the base path).
     *
     * This function is called after having evaluated itemPath == base to false;
     * also, 7-Zip reports folder paths without trailing separators.
     * Hence, the first case can be restricted further to only the case
     * where the base path has a trailing path separator: this means that if
     * the relative path is a single dot, the item must be a folder to be considered inside the base path.
     *
     * The remaining cases 2 and 3 are checked as follows:
     *   - Either the relative path doesn't start with two dots (and then the item is inside the base path) or
     *   - It starts with two dots, but it is not a dot-dot component (e.g., a normal filename starting with two dots).
     */
    return ( relativePath != BIT7Z_NATIVE_STRING( "." ) || isDir ) &&
           ( !starts_with( relativePath, BIT7Z_NATIVE_STRING( ".." ) ) ||
             ( relativePath.size() > 2 && !isPathSeparator( relativePath[ 2 ] ) ) );
}

void BitArchiveEditor::deleteItem( const tstring& itemPath, DeletePolicy policy ) {
    // The path to be deleted must be relative to the root of the archive.
    if ( itemPath.empty() || isPathSeparator( itemPath.front() ) ) {
        throw BitException( "Could not mark any path as deleted",
                            std::make_error_code( std::errc::invalid_argument ), itemPath );
    }

    bool deleted = false;

    // Normalized form of the path to be deleted inside the archive.
    const auto deletedPath = tstring_to_path( itemPath ).lexically_normal();
    for ( const auto& item: *inputArchive() ) {
        const fs::path path = item.nativePath();

        // The current item is marked as deleted if either:
        //  - it is lexicographically equivalent to the path to be deleted; or
        //  - we need to recursively delete directories,
        //    and the path of the item is (lexicographically) inside the path to be deleted.
        if ( path == deletedPath ||
             ( policy == DeletePolicy::RecurseDirs && isInsidePath( path, item.isDir(), deletedPath ) ) ) {
            markItemAsDeleted( item.index() );
            deleted = true;
        }
    }

    if ( !deleted ) {
        throw BitException( "Could not mark any path as deleted",
                            std::make_error_code( std::errc::no_such_file_or_directory ), itemPath );
    }
}

void BitArchiveEditor::markItemAsDeleted( uint32_t index ) {
    mEditedItems.erase( index );
    setDeletedIndex( index );
}

void BitArchiveEditor::setUpdateMode( UpdateMode mode ) {
    if ( mode == UpdateMode::None ) {
        throw BitException( "Cannot set update mode to UpdateMode::None",
                            std::make_error_code( std::errc::invalid_argument ) );
    }
    BitAbstractArchiveCreator::setUpdateMode( mode );
}

void BitArchiveEditor::applyChanges() {
    if ( !hasNewItems() && mEditedItems.empty() && !hasDeletedIndexes() ) {
        // Nothing to do here!
        return;
    }
    auto archivePath = inputArchive()->archivePath();
    compressTo( archivePath );
    mEditedItems.clear();
    setInputArchive( std::make_unique< BitInputArchive >( *this, archivePath, ArchiveStartOffset::FileStart ) );
}

auto BitArchiveEditor::findItem( const tstring& itemPath ) -> uint32_t {
    auto archiveItem = inputArchive()->find( itemPath );
    if ( archiveItem == inputArchive()->cend() ) {
        throw BitException( "Could not find the file in the archive",
                            std::make_error_code( std::errc::no_such_file_or_directory ), itemPath );
    }
    if ( isDeletedIndex( archiveItem->index() ) ) {
        throw BitException( "Could not find item",
                            make_error_code( BitError::ItemMarkedAsDeleted ), itemPath );
    }
    return archiveItem->index();
}

void BitArchiveEditor::checkIndex( uint32_t index ) {
    if ( index >= inputArchiveItemsCount() ) {
        throw BitException( "Cannot edit item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }
    if ( isDeletedIndex( index ) ) {
        throw BitException( "Cannot edit item at the index " + std::to_string( index ),
                            make_error_code( BitError::ItemMarkedAsDeleted ) );
    }
}

auto BitArchiveEditor::itemProperty( InputIndex index, BitProperty property ) const -> BitPropVariant {
    const auto mappedIndex = static_cast< uint32_t >( index );
    if ( mappedIndex < inputArchiveItemsCount() ) {
        auto res = mEditedItems.find( mappedIndex );
        if ( res != mEditedItems.end() ) {
            return res->second->itemProperty( property );
        }
        return inputArchive()->itemProperty( mappedIndex, property );
    }
    return BitOutputArchive::itemProperty( index, property );
}

auto BitArchiveEditor::itemStream( InputIndex index, ISequentialInStream** inStream ) const -> HRESULT {
    const auto mappedIndex = static_cast< uint32_t >( index );
    if ( mappedIndex < inputArchiveItemsCount() ) { //old item in the archive
        auto res = mEditedItems.find( mappedIndex );
        if ( res != mEditedItems.end() ) { //user wants to update the old item in the archive
            return res->second->getStream( inStream );
        }
        return S_OK;
    }
    return BitOutputArchive::itemStream( index, inStream );
}

auto BitArchiveEditor::hasNewData( uint32_t index ) const noexcept -> bool {
    const auto mappedIndex = static_cast< uint32_t >( itemInputIndex( index ) );
    if ( mappedIndex >= inputArchiveItemsCount() ) {
        return true; //new item
    }
    auto editedItem = mEditedItems.find( mappedIndex );
    if ( editedItem != mEditedItems.end() ) {
        return editedItem->second->hasNewData(); //renamed item -> false (no new data), updated item -> true
    }
    return false;
}

auto BitArchiveEditor::hasNewProperties( uint32_t index ) const noexcept -> bool {
    const auto mappedIndex = static_cast< uint32_t >( itemInputIndex( index ) );
    const bool isEditedItem = mEditedItems.find( mappedIndex ) != mEditedItems.end();
    return mappedIndex >= inputArchiveItemsCount() || isEditedItem;
}

} // namespace bit7z
