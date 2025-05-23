/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FSUTIL_HPP
#define FSUTIL_HPP

#include <string>

#include "bitdefines.hpp"
#include "bittypes.hpp"
#include "internal/fs.hpp"
#include "internal/windows.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

enum struct SymlinkPolicy {
    Follow,
    DoNotFollow
};

namespace fsutil {

BIT7Z_NODISCARD auto stem( const tstring& path ) -> tstring;

BIT7Z_NODISCARD auto extension( const fs::path& path ) -> tstring;

// Note: wildcard_match is "semi-public", so we cannot pass the path as fs::path!
BIT7Z_NODISCARD auto wildcard_match( const tstring& pattern, const tstring& path ) -> bool;

BIT7Z_NODISCARD auto get_file_attributes_ex( const fs::path& filePath,
                                             SymlinkPolicy symlinkPolicy,
                                             WIN32_FILE_ATTRIBUTE_DATA& fileMetadata ) noexcept -> bool;

#ifdef _WIN32
// TODO: In future, use std::optional instead of empty FILETIME objects.
auto set_file_time( const fs::path& filePath, FILETIME creation, FILETIME access, FILETIME modified ) noexcept -> bool;
#else
auto set_file_modified_time( const fs::path& filePath, FILETIME ftModified ) noexcept -> bool;
#endif

auto set_file_attributes( const fs::path& filePath, DWORD attributes ) noexcept -> bool;

BIT7Z_NODISCARD auto in_archive_path( const fs::path& filePath,
                                      const fs::path& searchPath = fs::path{} ) -> fs::path;

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )

BIT7Z_NODISCARD auto should_format_long_path( const fs::path& path ) -> bool;

BIT7Z_NODISCARD auto format_long_path( const fs::path& path ) -> fs::path;

#   define FORMAT_LONG_PATH( path ) \
        filesystem::fsutil::should_format_long_path( path ) ? filesystem::fsutil::format_long_path( path ) : path

#else
#   define FORMAT_LONG_PATH( path ) path
#endif

/**
 * @brief When writing multi-volume archives, we keep all the volume streams open until we finished.
 * This is less than ideal, and there's a limit in the number of open file descriptors/handles.
 * This function is a temporary workaround, where we increase such a limit to the maximum value allowed by the OS.
 */
void increase_opened_files_limit();

#if defined( _WIN32 ) && defined( BIT7Z_PATH_SANITIZATION )
/**
 * Sanitizes the given file path, removing any eventual Windows illegal character
 * (https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file)
 *
 * @param path The path to be sanitized.
 *
 * @return the sanitized path, where illegal characters are replaced with the '_' character.
 */
auto sanitize_path( const fs::path& path ) -> fs::path;

auto sanitized_extraction_path( const fs::path& outDir, const fs::path& itemPath ) -> fs::path;
#endif

}  // namespace fsutil
}  // namespace filesystem
}  // namespace bit7z

#endif // FSUTIL_HPP
