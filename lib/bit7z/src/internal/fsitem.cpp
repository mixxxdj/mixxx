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

#include <system_error>

#include "bitexception.hpp"
#include "internal/cfileinstream.hpp"
#include "internal/csymlinkinstream.hpp"
#include "internal/fsitem.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

/* NOTES:
 * 1) mPath contains the path to the file, including the filename. It can be relative or absolute, according to what
 *    the user passes as the path parameter in the constructor.
 * 2) mSearchPath contains the search path in which the item was found (e.g., if FilesystemIndexer is searching
 *    for items in "foo/bar/", each FilesystemItem created for the elements it found will have
 *    mSearchPath == "foo/bar"). As in mPath, mSearchPath does not contain trailing / or \! *
 * 3) mInArchivePath is the path of the item in the archive. If not already given (i.e., the user doesn't want to custom
 *    the path of the file in the archive), the path in the archive is calculated from mPath and mSearchPath
 *    (see inArchivePath() method). */

FilesystemItem::FilesystemItem( const fs::path& itemPath, fs::path inArchivePath, SymlinkPolicy symlinkPolicy )
    : mFileAttributeData(),
      mInArchivePath( !inArchivePath.empty() ? std::move( inArchivePath ) : fsutil::in_archive_path( itemPath ) ),
      mSymlinkPolicy{ symlinkPolicy } {
    std::error_code error;

    mFileEntry.assign( FORMAT_LONG_PATH( itemPath ), error );
    if ( error ) {
        throw BitException( "Cannot read file entry", error, path_to_tstring( itemPath ) );
    }
    if ( !mFileEntry.exists( error ) ) { // NOLINT
        if ( !error ) { // call to "exists(error)" succeeded
            error = std::make_error_code( std::errc::no_such_file_or_directory );
        }
        throw BitException( "Invalid path", error, path_to_tstring( itemPath ) );
    }
    initAttributes( mFileEntry.path() );
}

FilesystemItem::FilesystemItem( fs::directory_entry entry, const fs::path& searchPath, SymlinkPolicy symlinkPolicy )
    : mFileEntry( std::move( entry ) ),
      mFileAttributeData(),
      mInArchivePath( fsutil::in_archive_path( mFileEntry.path(), searchPath ) ),
      mSymlinkPolicy{ symlinkPolicy } {
    initAttributes( mFileEntry.path() );
}

void FilesystemItem::initAttributes( const fs::path& itemPath ) {
    if ( !fsutil::get_file_attributes_ex( itemPath.c_str(), mSymlinkPolicy, mFileAttributeData ) ) {
        //should not happen, but anyway...
        const auto error = last_error_code();
        throw BitException( "Could not retrieve file attributes", error, path_to_tstring( itemPath ) );
    }
}

auto FilesystemItem::isDots() const -> bool {
    const auto filename = mFileEntry.path().filename();
    return ( filename == "." || filename == ".." );
}

auto FilesystemItem::isDir() const noexcept -> bool {
    std::error_code error;
    const bool res = mFileEntry.is_directory( error );
    return !error && res;
}

auto FilesystemItem::size() const noexcept -> uint64_t {
    std::error_code error;
    if ( mSymlinkPolicy == SymlinkPolicy::DoNotFollow && isSymLink() ) {
        return fs::read_symlink( mFileEntry, error ).u8string().size();
    }
    const auto res = mFileEntry.file_size( error );
    return !error ? res : 0;
}

auto FilesystemItem::creationTime() const noexcept -> FILETIME {
    return mFileAttributeData.ftCreationTime;
}

auto FilesystemItem::lastAccessTime() const noexcept -> FILETIME {
    return mFileAttributeData.ftLastAccessTime;
}

auto FilesystemItem::lastWriteTime() const noexcept -> FILETIME {
    return mFileAttributeData.ftLastWriteTime;
}

auto FilesystemItem::name() const -> tstring {
    BIT7Z_MAYBE_UNUSED std::error_code error;
    return path_to_tstring( fs::canonical( mFileEntry, error ).filename() );
}

auto FilesystemItem::path() const -> tstring {
    return path_to_tstring( mFileEntry.path() );
}

/* Note: inArchivePath() returns the path that should be used inside the archive when compressing the item,
 * i.e., the path relative to the 'root' of the archive.
 * This is needed to behave like 7-zip and retain the directory structure when creating new archives.
 *
 * In particular, 7-zip behaves differently according to the kind of paths that are passed to it:
 * + Absolute paths (e.g. "C:\foo\bar\test.txt"):
 *   + The file is compressed without any directory structure (e.g., "test.txt"),
 *     unless it was inside a directory passed by the user and scanned by FilesystemIndexer:
 *     in this case, only the directory structure is retained.
 *
 * + Relative paths containing the current directory or outside references
 *   (e.g., containing a "./" or "../" substring, like in "../foo/bar/test.txt"):
 *   + Same as absolute paths (e.g., "test.txt").
 *
 * + Relative paths (e.g., "foo/bar/test.txt"):
 *   + The file is compressed retaining the directory structure (e.g., "foo/bar/test.txt" in both example cases).
 *
 * If the mInArchivePath is already given (i.e., the user wants a custom mapping of files), this one is returned.*/
auto FilesystemItem::inArchivePath() const -> fs::path {
    return mInArchivePath;
}

auto FilesystemItem::attributes() const noexcept -> uint32_t {
    return mFileAttributeData.dwFileAttributes;
}

auto FilesystemItem::getStream( ISequentialInStream** inStream ) const -> HRESULT {
    if ( isDir() ) {
        return S_OK;
    }

    if ( mSymlinkPolicy == SymlinkPolicy::DoNotFollow && isSymLink() ) {
        try {
            auto inStreamLoc = bit7z::make_com< CSymlinkInStream >( filesystemPath() );
            *inStream = inStreamLoc.Detach();
            return S_OK;
        } catch ( const BitException& ex ) {
            return ex.nativeCode();
        }
    }

    try {
        auto inStreamLoc = bit7z::make_com< CFileInStream >( filesystemPath() );
        *inStream = inStreamLoc.Detach();
    } catch ( const BitException& ex ) {
        return ex.nativeCode();
    }
    return S_OK;
}

auto FilesystemItem::filesystemPath() const -> const fs::path& {
    return mFileEntry.path();
}

auto FilesystemItem::filesystemName() const -> fs::path {
    BIT7Z_MAYBE_UNUSED std::error_code error;
    return fs::canonical( mFileEntry, error ).filename();
}

auto FilesystemItem::itemProperty( BitProperty property ) const -> BitPropVariant {
    std::error_code error;
    if ( property == BitProperty::SymLink && mFileEntry.is_symlink( error ) ) {
        const auto symlinkPath = fs::read_symlink( mFileEntry.path(), error );
        return !error ? BitPropVariant{ path_to_wide_string( symlinkPath ) } : BitPropVariant{};
    }
    return GenericInputItem::itemProperty( property );
}

auto FilesystemItem::isSymLink() const -> bool {
    BIT7Z_MAYBE_UNUSED std::error_code error;
    return mFileEntry.is_symlink( error );
}

} // namespace filesystem
} // namespace bit7z