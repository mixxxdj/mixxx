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

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitfilecompressor.hpp"
#include "bitoutputarchive.hpp"
#include "internal/fs.hpp"

using namespace std;
using namespace bit7z;

BitFileCompressor::BitFileCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitCompressor( lib, format ) {}

/* from filesystem to filesystem */

void BitFileCompressor::compress( const std::vector< tstring >& inPaths, const tstring& outFile ) const {
    if ( inPaths.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive outputArchive{ *this, outFile };
    outputArchive.addItems( inPaths );
    outputArchive.compressTo( outFile );
}

void BitFileCompressor::compress( const std::map< tstring, tstring >& inPaths, const tstring& outFile ) const {
    if ( inPaths.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive outputArchive{ *this, outFile };
    outputArchive.addItems( inPaths );
    outputArchive.compressTo( outFile );
}

void BitFileCompressor::compressFiles( const std::vector< tstring >& inFiles, const tstring& outFile ) const {
    if ( inFiles.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive outputArchive{ *this, outFile };
    outputArchive.addFiles( inFiles );
    outputArchive.compressTo( outFile );
}

void BitFileCompressor::compressFiles( const tstring& inDir, const tstring& outFile,
                                       bool recursive, const tstring& filter ) const {
    if ( !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive outputArchive{ *this, outFile };
    outputArchive.addFiles( inDir, filter, recursive );
    outputArchive.compressTo( outFile );
}

void BitFileCompressor::compressDirectory( const tstring& inDir, const tstring& outFile ) const {
    if ( !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive outputArchive{ *this, outFile };
    outputArchive.addDirectory( inDir );
    outputArchive.compressTo( outFile );
}

void BitFileCompressor::compressDirectoryContents( const tstring& inDir,
                                                   const tstring& outFile,
                                                   bool recursive,
                                                   const tstring& filter ) const {
    if ( !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive outputArchive{ *this, outFile };
    outputArchive.addDirectoryContents( inDir, filter, recursive );
    outputArchive.compressTo( outFile );
}

/* from filesystem to stream */

void BitFileCompressor::compress( const std::vector< tstring >& inPaths, std::ostream& outStream ) const {
    if ( inPaths.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive outputArchive{ *this };
    outputArchive.addItems( inPaths );
    outputArchive.compressTo( outStream );
}

void BitFileCompressor::compress( const std::map< tstring, tstring >& inPaths, std::ostream& outStream ) const {
    if ( inPaths.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive outputArchive{ *this };
    outputArchive.addItems( inPaths );
    outputArchive.compressTo( outStream );
}
