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

#include <algorithm> //for std::adjacent_find

#ifndef _WIN32
#include <sys/resource.h> // for rlimit, getrlimit, and setrlimit
#include <sys/stat.h>
#include <unistd.h>

#include "internal/dateutil.hpp"

// For some reason, GCC on macOS requires including <climits> for defining OPEN_MAX.
#if defined( __APPLE__ ) && !defined( __clang__ )
#include <climits>
#endif
#elif defined( BIT7Z_PATH_SANITIZATION )
#include <cwctype> // for iswdigit
#endif

#include "internal/fsutil.hpp"
#include "internal/stringutil.hpp"

using namespace std;

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

auto fsutil::stem( const tstring& path ) -> tstring {
    return path_to_tstring( tstring_to_path( path ).stem() );
}

auto fsutil::extension( const fs::path& path ) -> tstring {
    const fs::path ext = path.extension();
    if ( ext.empty() ) {
        return {};
    }

    // We don't want the leading dot of the extension!
    const tstring result = path_to_tstring( ext );
    return result.substr( 1 );
}

inline auto contains_dot_references( const fs::path& path ) -> bool {
    return std::find_if( path.begin(), path.end(), [] ( const fs::path& component ) -> bool {
        return component == BIT7Z_NATIVE_STRING( "." ) || component == BIT7Z_NATIVE_STRING( ".." );
    }) != path.end();
}

auto fsutil::in_archive_path( const fs::path& filePath, const fs::path& searchPath ) -> fs::path {
    /* Note: the following algorithm tries to emulate the behavior of 7-zip when dealing with
             paths of items in archives. */

    const auto& normalPath = filePath.lexically_normal();

    auto filename = normalPath.filename();
    if ( filename == BIT7Z_NATIVE_STRING( "." ) || filename == BIT7Z_NATIVE_STRING( ".." ) ) {
        return {};
    }
    if ( filename.empty() ) {
        filename = normalPath.parent_path().filename();
    }

    if ( filePath.is_absolute() || contains_dot_references( filePath ) ) {
        // Note: in this case, if the file was found while indexing a directory passed by the user, we need to retain
        // the internal structure of that folder (mSearchPath), otherwise we use only the file name.
        if ( searchPath.empty() ) {
            return filename;
        }
        return searchPath / filename;
    }

    // Here, the path is relative and without ./ or ../ => e.g. foo/bar/test.txt

    if ( !searchPath.empty() ) {
        // The item was found while indexing a directory
        return searchPath / filename;
    }
    return filePath;
}

// A modified version of the code found here: https://stackoverflow.com/a/3300547
auto w_match( tstring::const_iterator patternIt, // NOLINT(misc-no-recursion)
              const tstring::const_iterator& patternEnd,
              tstring::const_iterator strIt,
              const tstring::const_iterator& strEnd ) -> bool {
    for ( ; patternIt != patternEnd; ++patternIt ) {
        switch ( *patternIt ) {
            case BIT7Z_STRING( '?' ):
                if ( strIt == strEnd ) {
                    return false;
                }
                ++strIt;
                break;
            case BIT7Z_STRING( '*' ): {
                while ( patternIt + 1 != patternEnd && *( patternIt + 1 ) == '*' ) {
                    ++patternIt;
                }
                if ( patternIt + 1 == patternEnd ) {
                    return true;
                }
                for ( auto i = strIt; i != strEnd; ++i ) {
                    if ( w_match( patternIt + 1, patternEnd, i, strEnd ) ) {
                        return true;
                    }
                }
                return false;
            }
            default:
                if ( strIt == strEnd || *strIt != *patternIt ) {
                    return false;
                }
                ++strIt;
        }
    }
    return strIt == strEnd;
}

auto fsutil::wildcard_match( const tstring& pattern, const tstring& str ) -> bool { // NOLINT(misc-no-recursion)
    if ( pattern.empty() ) {
        return wildcard_match( BIT7Z_STRING( "*" ), str );
    }
    return w_match( pattern.cbegin(), pattern.cend(), str.begin(), str.end() );
}

#ifndef _WIN32

auto restore_symlink( const std::string& name ) -> bool {
    std::ifstream ifs( name, std::ios::in | std::ios::binary );
    if ( !ifs.is_open() ) {
        return false;
    }

    // Reading the path stored in the link file.
    std::string linkPath;
    linkPath.resize( MAX_PATHNAME_LEN );
    ifs.getline( &linkPath[ 0 ], MAX_PATHNAME_LEN ); // NOLINT(readability-container-data-pointer)

    if ( !ifs ) { // Error while reading the path, exiting.
        return false;
    }

    // Shrinking the path string to its actual size.
    linkPath.resize( static_cast< size_t >( ifs.gcount() ) );

    // No need to keep the file open.
    ifs.close();

    // Removing the link file.
    std::error_code error;
    fs::remove( name, error );

    // Restoring the symbolic link to the target file.
    return !error && symlink( linkPath.c_str(), name.c_str() ) == 0;
}

static const mode_t global_umask = []() noexcept -> mode_t {
    // Getting and setting the current umask.
    // Note: flawfinder warns about umask with the mask set to 0;
    // however, we use it only to read the current umask,
    // then we restore the old value, hence we can ignore the warning.
    const mode_t currentUmask{ umask( 0 ) }; // flawfinder: ignore

    // Restoring the umask.
    umask( currentUmask ); // flawfinder: ignore

    return static_cast< mode_t >( static_cast< int >( fs::perms::all ) & ( ~currentUmask ) );
}();

#endif

#ifndef _WIN32
#if defined( __APPLE__ ) || defined( BSD ) || \
    defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __DragonFly__ )
using stat_t = struct stat;
const auto os_lstat = &lstat;
const auto os_stat = &stat;
#else
using stat_t = struct stat64;
const auto os_lstat = &lstat64;
const auto os_stat = &stat64;
#endif
#endif

auto fsutil::set_file_attributes( const fs::path& filePath, DWORD attributes ) noexcept -> bool {
    if ( filePath.empty() ) {
        return false;
    }

#ifdef _WIN32
    return ::SetFileAttributesW( filePath.c_str(), attributes ) != FALSE;
#else
    stat_t fileStat{};
    if ( os_lstat( filePath.c_str(), &fileStat ) != 0 ) {
        return false;
    }

    if ( ( attributes & FILE_ATTRIBUTE_UNIX_EXTENSION ) != 0 ) {
        fileStat.st_mode = static_cast< mode_t >( attributes >> 16U );
        if ( S_ISLNK( fileStat.st_mode ) ) {
            return restore_symlink( filePath );
        }

        if ( S_ISDIR( fileStat.st_mode ) ) {
            fileStat.st_mode |= ( S_IRUSR | S_IWUSR | S_IXUSR );
        } else if ( !S_ISREG( fileStat.st_mode ) ) {
            return true;
        }
    } else if ( S_ISLNK( fileStat.st_mode ) ) {
        return true;
    } else if ( !S_ISDIR( fileStat.st_mode ) && ( attributes & FILE_ATTRIBUTE_READONLY ) != 0 ) {
        fileStat.st_mode &= static_cast< mode_t >( ~( S_IWUSR | S_IWGRP | S_IWOTH ) );
    }

    const fs::perms filePermissions = static_cast< fs::perms >( fileStat.st_mode & global_umask ) & fs::perms::mask;
    std::error_code error;
    fs::permissions( filePath, filePermissions, error );
    return !error;
#endif
}

#ifdef _WIN32
auto fsutil::set_file_time( const fs::path& filePath,
                            FILETIME creation,
                            FILETIME access,
                            FILETIME modified ) noexcept -> bool {
    if ( filePath.empty() ) {
        return false;
    }

    bool res = false;
    HANDLE hFile = ::CreateFileW( filePath.c_str(),
                                  GENERIC_READ | FILE_WRITE_ATTRIBUTES,
                                  FILE_SHARE_READ,
                                  nullptr,
                                  OPEN_EXISTING,
                                  0,
                                  nullptr );
    if ( hFile != INVALID_HANDLE_VALUE ) { // NOLINT(cppcoreguidelines-pro-type-cstyle-cast,performance-no-int-to-ptr)
        res = ::SetFileTime( hFile, &creation, &access, &modified ) != FALSE;
        CloseHandle( hFile );
    }
    return res;
}
#else
auto fsutil::set_file_modified_time( const fs::path& filePath, FILETIME ftModified ) noexcept -> bool {
    if ( filePath.empty() ) {
        return false;
    }

    std::error_code error;
    auto fileTime = FILETIME_to_file_time_type( ftModified );
    fs::last_write_time( filePath, fileTime, error );
    return !error;
}
#endif

auto fsutil::get_file_attributes_ex( const fs::path& filePath,
                                     SymlinkPolicy symlinkPolicy,
                                     WIN32_FILE_ATTRIBUTE_DATA& fileMetadata ) noexcept -> bool {
    if ( filePath.empty() ) {
        return false;
    }

#ifdef _WIN32
    (void)symlinkPolicy;
    return ::GetFileAttributesExW( filePath.c_str(), GetFileExInfoStandard, &fileMetadata ) != FALSE;
#else
    stat_t statInfo{};
    const auto statRes = symlinkPolicy == SymlinkPolicy::Follow ?
                         os_stat( filePath.c_str(), &statInfo ) :
                         os_lstat( filePath.c_str(), &statInfo );
    if ( statRes != 0 ) {
        return false;
    }

    // File attributes
    fileMetadata.dwFileAttributes = S_ISDIR( statInfo.st_mode ) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_ARCHIVE;
    if ( ( statInfo.st_mode & S_IWUSR ) == 0 ) {
        fileMetadata.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }
    constexpr auto kMask = 0xFFFFu;
    std::uint32_t unixAttributes = ( ( statInfo.st_mode & kMask ) << 16u );
    fileMetadata.dwFileAttributes |= FILE_ATTRIBUTE_UNIX_EXTENSION + unixAttributes;

    // File times
    fileMetadata.ftCreationTime = time_to_FILETIME( statInfo.st_ctime );
    fileMetadata.ftLastAccessTime = time_to_FILETIME( statInfo.st_atime );
    fileMetadata.ftLastWriteTime = time_to_FILETIME( statInfo.st_mtime );
    return true;
#endif
}

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )

namespace {
constexpr auto kLongPathPrefix = BIT7Z_NATIVE_STRING( R"(\\?\)" );
} // namespace

auto fsutil::should_format_long_path( const fs::path& path ) -> bool {
    constexpr auto kMaxDosFilenameSize = 12;

    if ( !path.is_absolute() ) {
        return false;
    }
    const auto& pathStr = path.native();
    if ( pathStr.size() < static_cast<std::size_t>( MAX_PATH - kMaxDosFilenameSize ) ) {
        return false;
    }
    return !starts_with( pathStr, kLongPathPrefix );
}

auto fsutil::format_long_path( const fs::path& path ) -> fs::path {
    fs::path longPath = kLongPathPrefix;
    // Note: we call this function after checking if we should format the given path as a long path.
    // This means that if the path starts with the \\ prefix,
    // it is a UNC path and not a long path prefixed with \\?\.
    if ( starts_with( path.native(), BIT7Z_NATIVE_STRING( R"(\\)" ) ) ) {
        longPath += L"UNC\\";
    }
    longPath += path;
    return longPath;
}

#endif

void fsutil::increase_opened_files_limit() {
#if defined( _MSC_VER )
    // http://msdn.microsoft.com/en-us/library/6e3b887c.aspx
    _setmaxstdio( 8192 );
#elif defined( __MINGW32__ )
    // MinGW uses an older max value for this function
    _setmaxstdio( 2048 );
#else
    rlimit limits{ 0, 0 };
    if ( getrlimit( RLIMIT_NOFILE, &limits ) == 0 ) {
#ifdef __APPLE__
        limits.rlim_cur = std::min( static_cast< rlim_t >( OPEN_MAX ), limits.rlim_max );
#else
        limits.rlim_cur = limits.rlim_max;
#endif
        setrlimit( RLIMIT_NOFILE, &limits );
    }
#endif
}

#if defined( _WIN32 ) && defined( BIT7Z_PATH_SANITIZATION )
namespace {
auto is_windows_reserved_name( const std::wstring& component ) -> bool {
    // Reserved file names that can't be used on Windows: CON, PRN, AUX, and NUL.
    if ( component == L"CON" || component == L"PRN" || component == L"AUX" || component == L"NUL" ) {
        return true;
    }
    // Reserved file names that can't be used on Windows:
    // COM0, COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9,
    // LPT0, LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, and LPT9.
    constexpr auto reserved_component_size = 4;
    return component.size() == reserved_component_size &&
           ( component.rfind( L"COM", 0 ) == 0 || component.rfind( L"LPT", 0 ) == 0 ) &&
           std::iswdigit( component.back() ) != 0;
}

auto sanitize_path_component( std::wstring component ) -> std::wstring {
    const auto firstNonSlash = component.find_first_not_of( L"/\\" );
    if ( firstNonSlash == std::wstring::npos ) {
        return L"";
    }
    if ( firstNonSlash != 0 ) {
        component.erase( 0, firstNonSlash );
    }

    // If the component is a reserved name on Windows, we prepend it with a '_' character.
    if ( is_windows_reserved_name( component ) ) {
        component.insert( 0, 1, L'_' );
        return component;
    }

    // Replacing all reserved characters in the component with the '_' character.
    std::replace_if( component.begin(), component.end(), []( wchar_t chr ) {
        constexpr auto kLastNonPrintableAscii = 31;
        return chr <= kLastNonPrintableAscii || chr == L'<' || chr == L'>' || chr == L':' ||
               chr == L'"' || chr == L'/' || chr == L'|' || chr == L'?' || chr == L'*';
    }, L'_' );
    return component;
}
} // namespace

auto fsutil::sanitize_path( const fs::path& path ) -> fs::path {
    if ( path == L"/" ) {
        return L"_";
    }

    fs::path sanitizedPath;
    for( const auto& pathComponent : path ) {
        // cppcheck-suppress useStlAlgorithm
        sanitizedPath /= sanitize_path_component( pathComponent.wstring() );
    }
    return sanitizedPath;
}

auto fsutil::sanitized_extraction_path( const fs::path& outDir, const fs::path& itemPath ) -> fs::path {
    return outDir / sanitize_path( itemPath );
}
#endif

} // namespace filesystem
} // namespace bit7z