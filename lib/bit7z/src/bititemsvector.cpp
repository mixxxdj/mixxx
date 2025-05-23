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

#include "bitexception.hpp"
#include "bititemsvector.hpp"
#include "internal/bufferitem.hpp"
#include "internal/fsindexer.hpp"
#include "internal/stdinputitem.hpp"
#include "internal/stringutil.hpp"

using namespace bit7z;
using filesystem::FilesystemItem;
using filesystem::FilesystemIndexer;
using filesystem::SymlinkPolicy;

void BitItemsVector::indexDirectory( const fs::path& inDir,
                                     const tstring& filter,
                                     FilterPolicy policy,
                                     IndexingOptions options ) {
    const auto symlinkPolicy = options.followSymlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    // Note: if inDir is an invalid path, FilesystemItem constructor throws a BitException!
    const FilesystemItem dirItem{ inDir, options.retainFolderStructure ? inDir : fs::path{}, symlinkPolicy };
    if ( filter.empty() && !dirItem.inArchivePath().empty() ) {
        mItems.emplace_back( std::make_unique< FilesystemItem >( dirItem ) );
    }
    FilesystemIndexer indexer{ dirItem, filter, policy, symlinkPolicy, options.onlyFiles };
    indexer.listDirectoryItems( mItems, options.recursive );
}

void BitItemsVector::indexPaths( const std::vector< tstring >& inPaths, IndexingOptions options ) {
    const auto symlinkPolicy = options.followSymlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    for ( const auto& inputPath : inPaths ) {
        const auto filePath = tstring_to_path( inputPath );
        const FilesystemItem item{ filePath,
                                   options.retainFolderStructure ? filePath : fs::path{},
                                   symlinkPolicy };
        indexItem( item, options );
    }
}

void BitItemsVector::indexPathsMap( const std::map< tstring, tstring >& inPaths, IndexingOptions options ) {
    const auto symlinkPolicy = options.followSymlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    for ( const auto& filePair : inPaths ) {
        const FilesystemItem item{ tstring_to_path( filePair.first ), tstring_to_path( filePair.second ), symlinkPolicy };
        indexItem( item, options );
    }
}

void BitItemsVector::indexItem( const FilesystemItem& item, IndexingOptions options ) {
    if ( !item.isDir() ) {
        mItems.emplace_back( std::make_unique< FilesystemItem >( item ) );
    } else if ( options.recursive ) { // The item is a directory
        if ( !item.inArchivePath().empty() ) {
            mItems.emplace_back( std::make_unique< FilesystemItem >( item ) );
        }
        const auto symlinkPolicy = options.followSymlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
        FilesystemIndexer indexer{ item, {}, FilterPolicy::Include, symlinkPolicy, options.onlyFiles };
        indexer.listDirectoryItems( mItems, true );
    } else {
        // No action needed
    }
}

void BitItemsVector::indexFile( const tstring& inFile, const tstring& name, bool followSymlinks ) {
    const fs::path filePath = tstring_to_path( inFile );
    if ( fs::is_directory( filePath ) ) {
        throw BitException( "Input path points to a directory, not a file",
                            std::make_error_code( std::errc::invalid_argument ), inFile );
    }
    const auto symlinkPolicy = followSymlinks ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    mItems.emplace_back( std::make_unique< FilesystemItem >( filePath, tstring_to_path( name ), symlinkPolicy ) );
}

void BitItemsVector::indexBuffer( const vector< byte_t >& inBuffer, const tstring& name ) {
    mItems.emplace_back( std::make_unique< BufferItem >( inBuffer, tstring_to_path( name ) ) );
}

void BitItemsVector::indexStream( std::istream& inStream, const tstring& name ) {
    mItems.emplace_back( std::make_unique< StdInputItem >( inStream, tstring_to_path( name ) ) );
}

auto BitItemsVector::size() const -> size_t {
    return mItems.size();
}

auto BitItemsVector::operator[]( GenericInputItemVector::size_type index ) const -> const GenericInputItem& {
    // Note: here index is expected to be correct!
    return *mItems[ index ];
}

auto BitItemsVector::begin() const noexcept -> GenericInputItemVector::const_iterator {
    return mItems.cbegin();
}

auto BitItemsVector::end() const noexcept -> GenericInputItemVector::const_iterator {
    return mItems.cend();
}

auto BitItemsVector::cbegin() const noexcept -> GenericInputItemVector::const_iterator {
    return mItems.cbegin();
}

auto BitItemsVector::cend() const noexcept -> GenericInputItemVector::const_iterator {
    return mItems.cend();
}

/* Note: separate declaration/definition of the default destructor is needed to use an incomplete type
 *       for the unique_ptr objects stored in the vector. */
BitItemsVector::~BitItemsVector() = default;