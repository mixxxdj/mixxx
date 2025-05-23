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

#include "bit7zlibrary.hpp"
#include "bitexception.hpp"
#include "bitformat.hpp"
#include "internal/com.hpp"
#include "internal/guids.hpp"
#include "internal/stringutil.hpp"

#include <7zip/Archive/IArchive.h>

#ifdef _WIN32
#   define Bit7zLoadLibrary( lib_name ) LoadLibraryW( WIDEN( (lib_name) ).c_str() )
#   define ERROR_CODE( errc ) bit7z::last_error_code()
#else
#   include <dlfcn.h>

#   define Bit7zLoadLibrary( lib_name ) dlopen( (lib_name).c_str(), RTLD_LAZY )
#   define GetProcAddress dlsym
#   define FreeLibrary dlclose
#   define ERROR_CODE( errc ) std::make_error_code( errc )  //same behavior as boost::shared_library
#endif

using namespace bit7z;

Bit7zLibrary::Bit7zLibrary( const tstring& libraryPath ) : mLibrary( Bit7zLoadLibrary( libraryPath ) ) {
    if ( mLibrary == nullptr ) {
        const auto error = ERROR_CODE( std::errc::bad_file_descriptor );
        throw BitException( "Failed to load the 7-zip library", error );
    }

    mCreateObjectFunc = GetProcAddress( mLibrary, "CreateObject" );

    if ( mCreateObjectFunc == nullptr ) {
        FreeLibrary( mLibrary );
        const auto error = ERROR_CODE( std::errc::invalid_seek );
        throw BitException( "Failed to get CreateObject function", error );
    }
}

Bit7zLibrary::~Bit7zLibrary() {
    FreeLibrary( mLibrary );
}

void Bit7zLibrary::setLargePageMode() {
    using SetLargePageMode = HRESULT ( WINAPI* )();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto pSetLargePageMode = reinterpret_cast< SetLargePageMode >( GetProcAddress( mLibrary, "SetLargePageMode" ) );
    if ( pSetLargePageMode == nullptr ) {
        const auto error = ERROR_CODE( std::errc::invalid_seek );
        throw BitException( "Failed to get SetLargePageMode function", error );
    }
    const HRESULT res = pSetLargePageMode();
    if ( res != S_OK ) {
        throw BitException( "Failed to set the large page mode", make_hresult_code( res ) );
    }
}

using CreateObjectFunc = HRESULT ( WINAPI* )( const GUID* clsID, const GUID* interfaceID, void** out );

// Making the code not build when choosing a wrong interface type (only IInArchive and IOutArchive are supported!).
// Note: use template variables once we drop support to GCC 4.9.
template< typename T >
constexpr auto interface_id() -> const GUID&;

template<>
constexpr auto interface_id< IInArchive >() -> const GUID& {
    return bit7z::IID_IInArchive;
}

template<>
constexpr auto interface_id< IOutArchive >() -> const GUID& {
    return bit7z::IID_IOutArchive;
}

template< typename T >
BIT7Z_NODISCARD
auto create_archive_object( FARPROC creatorFunction, const BitInFormat& format, T** object ) -> HRESULT {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto createObject = reinterpret_cast< CreateObjectFunc >( creatorFunction );
    const auto formatID = format_guid( format );
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return createObject( &formatID, &interface_id< T >(), reinterpret_cast< void** >( object ) );
}

BIT7Z_NODISCARD
auto Bit7zLibrary::initInArchive( const BitInFormat& format ) const -> CMyComPtr< IInArchive > {
    CMyComPtr< IInArchive > inArchive{};
    const HRESULT res = create_archive_object( mCreateObjectFunc, format, &inArchive );
    if ( res != S_OK || inArchive == nullptr ) {
        throw BitException( "Failed to initialize the input archive object", make_hresult_code( res ) );
    }
    return inArchive;
}

BIT7Z_NODISCARD
auto Bit7zLibrary::initOutArchive( const BitInOutFormat& format ) const -> CMyComPtr< IOutArchive > {
    CMyComPtr< IOutArchive > outArchive{};
    const HRESULT res = create_archive_object( mCreateObjectFunc, format, &outArchive );
    if ( res != S_OK || outArchive == nullptr ) {
        throw BitException( "Failed to initialize the output archive object", make_hresult_code( res ) );
    }
    return outArchive;
}
