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

#include "bitformat.hpp"

using namespace std;

namespace bit7z {

namespace BitFormat {
#ifdef BIT7Z_AUTO_FORMAT
    const BitInFormat Auto( 0x00 );
#endif
    const BitInOutFormat Zip( 0x01, BIT7Z_STRING( ".zip" ),
                              BitCompressionMethod::Deflate,
                              FormatFeatures::MultipleFiles | FormatFeatures::CompressionLevel |
                              FormatFeatures::Encryption | FormatFeatures::MultipleMethods );
    const BitInOutFormat BZip2( 0x02, BIT7Z_STRING( ".bz2" ),
                                BitCompressionMethod::BZip2,
                                FormatFeatures::CompressionLevel );
    const BitInFormat Rar( 0x03 );
    const BitInFormat Arj( 0x04 ); //-V112
    const BitInFormat Z( 0x05 ); // NOLINT(*-identifier-length)
    const BitInFormat Lzh( 0x06 );
    const BitInOutFormat SevenZip( 0x07, BIT7Z_STRING( ".7z" ),
                                   BitCompressionMethod::Lzma2,
                                   FormatFeatures::MultipleFiles | FormatFeatures::SolidArchive |
                                   FormatFeatures::CompressionLevel | FormatFeatures::Encryption |
                                   FormatFeatures::HeaderEncryption | FormatFeatures::MultipleMethods );
    const BitInFormat Cab( 0x08 );
    const BitInFormat Nsis( 0x09 );
    const BitInFormat Lzma( 0x0A );
    const BitInFormat Lzma86( 0x0B );
    const BitInOutFormat Xz( 0x0C, BIT7Z_STRING( ".xz" ), // NOLINT(*-identifier-length)
                             BitCompressionMethod::Lzma2,
                             FormatFeatures::CompressionLevel );
    const BitInFormat Ppmd( 0x0D );
    const BitInFormat Zstd( 0x0E );
    const BitInFormat Vhdx( 0xC4 );
    const BitInFormat COFF( 0xC6 );
    const BitInFormat Ext( 0xC7 );
    const BitInFormat VMDK( 0xC8 );
    const BitInFormat VDI( 0xC9 );
    const BitInFormat QCow( 0xCA );
    const BitInFormat GPT( 0xCB );
    const BitInFormat Rar5( 0xCC );
    const BitInFormat IHex( 0xCD );
    const BitInFormat Hxs( 0xCE );
    const BitInFormat TE( 0xCF ); // NOLINT(*-identifier-length)
    const BitInFormat UEFIc( 0xD0 );
    const BitInFormat UEFIs( 0xD1 );
    const BitInFormat SquashFS( 0xD2 );
    const BitInFormat CramFS( 0xD3 );
    const BitInFormat APM( 0xD4 );
    const BitInFormat Mslz( 0xD5 );
    const BitInFormat Flv( 0xD6 );
    const BitInFormat Swf( 0xD7 );
    const BitInFormat Swfc( 0xD8 );
    const BitInFormat Ntfs( 0xD9 );
    const BitInFormat Fat( 0xDA );
    const BitInFormat Mbr( 0xDB );
    const BitInFormat Vhd( 0xDC );
    const BitInFormat Pe( 0xDD ); // NOLINT(*-identifier-length)
    const BitInFormat Elf( 0xDE );
    const BitInFormat Macho( 0xDF );
    const BitInFormat Udf( 0xE0 );
    const BitInFormat Xar( 0xE1 );
    const BitInFormat Mub( 0xE2 );
    const BitInFormat Hfs( 0xE3 );
    const BitInFormat Dmg( 0xE4 );
    const BitInFormat Compound( 0xE5 );
    const BitInOutFormat Wim( 0xE6, BIT7Z_STRING( ".wim" ),
                              BitCompressionMethod::Copy,
                              FormatFeatures::MultipleFiles );
    const BitInFormat Iso( 0xE7 );
    const BitInFormat Chm( 0xE9 );
    const BitInFormat Split( 0xEA );
    const BitInFormat Rpm( 0xEB );
    const BitInFormat Deb( 0xEC );
    const BitInFormat Cpio( 0xED );
    const BitInOutFormat Tar( 0xEE, BIT7Z_STRING( ".tar" ),
                              BitCompressionMethod::Copy,
                              FormatFeatures::MultipleFiles );
    const BitInOutFormat GZip( 0xEF, BIT7Z_STRING( ".gz" ),
                               BitCompressionMethod::Deflate,
                               FormatFeatures::CompressionLevel );
} // namespace BitFormat

auto BitInFormat::value() const noexcept -> unsigned char {
    return mValue;
}

auto BitInFormat::operator==( const BitInFormat& other ) const noexcept -> bool {
    return mValue == other.value();
}

auto BitInFormat::operator!=( const BitInFormat& other ) const noexcept -> bool {
    return !( *this == other );
}

auto BitInOutFormat::extension() const noexcept -> const tchar* {
    return mExtension;
}

auto BitInOutFormat::features() const noexcept -> FormatFeatures {
    return mFeatures;
}

auto BitInOutFormat::hasFeature( FormatFeatures feature ) const noexcept -> bool {
    return ( mFeatures & feature ) != 0;
}

auto BitInOutFormat::defaultMethod() const noexcept -> BitCompressionMethod {
    return mDefaultMethod;
}

}  // namespace bit7z

