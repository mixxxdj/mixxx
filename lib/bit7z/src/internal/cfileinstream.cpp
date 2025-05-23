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
#include "internal/cfileinstream.hpp"
#include "internal/stringutil.hpp"

namespace bit7z {

CFileInStream::CFileInStream( const fs::path& filePath ) : CStdInStream( mFileStream ) {
    /* Disabling std::ifstream's buffering, as unbuffered IO gives better performance
     * with the block sizes read/written by 7-Zip.
     * Note: we need to do this before and after opening the file (https://stackoverflow.com/a/59161297/3497024). */
    mFileStream.rdbuf()->pubsetbuf( nullptr, 0 );
    openFile( filePath );
// Unbuffered streams are slow for Visual Studio 2015
#if !defined(_MSC_VER) || _MSC_VER != 1900
    mFileStream.rdbuf()->pubsetbuf( nullptr, 0 );
#endif
}

void CFileInStream::openFile( const fs::path& filePath ) {
    mFileStream.open( filePath, std::ios::in | std::ios::binary ); // flawfinder: ignore
    if ( mFileStream.fail() ) {
#if defined( __MINGW32__ ) || defined( __MINGW64__ )
        std::error_code error{ errno, std::generic_category() };
#else
        const auto error = last_error_code();
#endif
        throw BitException( "Failed to open the archive file", error, path_to_tstring( filePath ) );
    }
}

} // namespace bit7z