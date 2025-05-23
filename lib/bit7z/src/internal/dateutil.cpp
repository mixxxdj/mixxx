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

namespace bit7z {
// 100ns intervals
using FileTimeTickRate = std::ratio< 1, 10'000'000 >; // NOLINT(*-magic-numbers)
// FileTimeDuration has the same layout as FILETIME;
using FileTimeDuration = std::chrono::duration< int64_t, FileTimeTickRate >;
// Seconds between 01/01/1601 (NT epoch) and 01/01/1970 (Unix epoch):
constexpr std::chrono::seconds nt_to_unix_epoch{ -11644473600 };

#if defined( BIT7Z_USE_STANDARD_FILESYSTEM ) && defined( __GLIBCXX__ )
// Seconds between 01/01/1970 (Unix epoch) and 01/01/2174 (libstdc++'s file_clock epoch).
constexpr std::chrono::seconds libstdcpp_file_clock_epoch{ -6437664000 };
#endif

#ifndef _WIN32
auto FILETIME_to_file_time_type( FILETIME fileTime ) -> fs::file_time_type {
    const FileTimeDuration fileTimeDuration{
        ( static_cast< std::uint64_t >( fileTime.dwHighDateTime ) << 32ull ) + fileTime.dwLowDateTime
    };

    const auto unixFileTime = fileTimeDuration + nt_to_unix_epoch;
    const auto systemFileTime = std::chrono::duration_cast< fs::file_time_type::clock::duration >( unixFileTime );
#if defined( BIT7Z_USE_STANDARD_FILESYSTEM ) && defined( __GLIBCXX__ )
    return fs::file_time_type{ systemFileTime + libstdcpp_file_clock_epoch };
#else
    return fs::file_time_type{ systemFileTime };
#endif
}

auto time_to_FILETIME( std::time_t value ) -> FILETIME {
    // NOLINTNEXTLINE(*-magic-numbers)
    const std::uint64_t timeInSeconds = ( static_cast< std::uint64_t >( value ) * 10000000ull ) + 116444736000000000ull;
    FILETIME fileTime{};
    fileTime.dwLowDateTime = static_cast< DWORD >( timeInSeconds );
    fileTime.dwHighDateTime = static_cast< DWORD >( timeInSeconds >> 32ull );
    return fileTime;
}

#endif

auto FILETIME_to_time_type( FILETIME fileTime ) -> time_type {
    const FileTimeDuration fileTimeDuration{
        ( static_cast< std::uint64_t >( fileTime.dwHighDateTime ) << 32ull ) + fileTime.dwLowDateTime
    };

    const auto unixEpoch = fileTimeDuration + nt_to_unix_epoch;
    return time_type{ std::chrono::duration_cast< std::chrono::system_clock::duration >( unixEpoch ) };
}

auto current_file_time() -> FILETIME {
#ifdef _WIN32
    FILETIME fileTime{};
    SYSTEMTIME systemTime{};

    GetSystemTime( &systemTime ); // Getting the current time as a SYSTEMTIME struct.
    SystemTimeToFileTime( &systemTime, &fileTime ); // Converting it to the FILETIME struct format.
    return fileTime;
#else
    auto currentTime = std::chrono::system_clock::now();
    const std::time_t timeValue = std::chrono::system_clock::to_time_t( currentTime );
    return time_to_FILETIME( timeValue );
#endif
}
}  // namespace bit7z

//#endif