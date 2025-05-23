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
#include "internal/fsindexer.hpp"
#include "internal/fsutil.hpp"
#include "internal/genericinputitem.hpp"
#include "internal/stringutil.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

FilesystemIndexer::FilesystemIndexer( FilesystemItem directory,
                                      tstring filter,
                                      FilterPolicy policy,
                                      SymlinkPolicy symlinkPolicy,
                                      bool onlyFiles )
    : mDirItem{ std::move( directory ) },
      mFilter{ std::move( filter ) },
      mPolicy{ policy },
      mSymlinkPolicy{ symlinkPolicy },
      mOnlyFiles{ onlyFiles } {
    if ( !mDirItem.isDir() ) {
        throw BitException( "Invalid path", std::make_error_code( std::errc::not_a_directory ), mDirItem.name() );
    }
}

namespace {
inline auto countItemsInPath( const fs::path& path ) -> std::size_t {
    std::error_code error;
    auto begin = fs::recursive_directory_iterator{ path, fs::directory_options::skip_permission_denied, error };
    return error ? 0 : static_cast< std::size_t >( std::distance( begin, fs::recursive_directory_iterator{} ) );
}
} // namespace

// NOTE: It indexes all the items whose metadata are needed in the archive to be created!
void FilesystemIndexer::listDirectoryItems( std::vector< std::unique_ptr< GenericInputItem > >& result,
                                            bool recursive ) {
    const bool includeRootPath = mFilter.empty() ||
                                 !mDirItem.filesystemPath().has_parent_path() ||
                                 mDirItem.inArchivePath().filename() != mDirItem.filesystemName();
    const bool shouldIncludeMatchedItems = mPolicy == FilterPolicy::Include;

    const fs::path basePath = mDirItem.filesystemPath();
    std::error_code error;
    result.reserve( result.size() + countItemsInPath( basePath ) );
    for ( auto iterator = fs::recursive_directory_iterator{ basePath, fs::directory_options::skip_permission_denied, error };
          iterator != fs::recursive_directory_iterator{};
          ++iterator ) {
        const auto& currentEntry = *iterator;
        const auto& itemPath = currentEntry.path();

        const auto itemIsDir = currentEntry.is_directory( error );
        const auto itemName = path_to_tstring( itemPath.filename() );

        /* An item matches if:
         *  - Its name matches the wildcard pattern, and
         *  - Either is a file, or we are interested also to include folders in the index.
         *
         * Note: The boolean expression uses short-circuiting to optimize the evaluation. */
        const bool itemMatches = ( !mOnlyFiles || !itemIsDir ) && fsutil::wildcard_match( mFilter, itemName );
        if ( itemMatches == shouldIncludeMatchedItems ) {
            const auto prefix = fs::relative( itemPath, basePath, error ).remove_filename();
            const auto searchPath = includeRootPath ? mDirItem.inArchivePath() / prefix : prefix;
            result.emplace_back( std::make_unique< FilesystemItem >( currentEntry, searchPath, mSymlinkPolicy ) );
        }

        /* We don't need to recurse inside the current item if:
         *  - it is not a directory; or
         *  - we are not indexing recursively, and the directory's name doesn't match the wildcard filter. */
        if ( !itemIsDir || ( !recursive && ( itemMatches != shouldIncludeMatchedItems ) ) ) {
            iterator.disable_recursion_pending();
        }
    }
}

} // namespace filesystem
} // namespace bit7z