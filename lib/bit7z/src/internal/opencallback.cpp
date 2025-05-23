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
#include "internal/opencallback.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"

#include <utility>

namespace bit7z {

OpenCallback::OpenCallback( const BitAbstractArchiveHandler& handler, fs::path archivePath )
    : Callback( handler ),
      mSubArchiveMode( false ),
      mArchivePath{ std::move( archivePath ) },
      mPasswordWasAsked{ false } {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::SetTotal( const UInt64* /* files */, const UInt64* /* bytes */ ) noexcept {
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::SetCompleted( const UInt64* /* files */, const UInt64* /* bytes */ ) noexcept {
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::GetProperty( PROPID property, PROPVARIANT* value ) noexcept try {
    BitPropVariant prop;
    if ( property == kpidName ) {
        prop = mSubArchiveMode ? mSubArchiveName : path_to_wide_string( mArchivePath.filename() );
    }
    *value = prop;
    prop.bstrVal = nullptr; // NOLINT(*-pro-type-union-access)
    return S_OK;
} catch ( const BitException& ex ) {
    return ex.hresultCode();
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::GetStream( const wchar_t* name, IInStream** inStream ) noexcept {
    try {
        *inStream = nullptr;
        if ( mSubArchiveMode ) {
            return S_FALSE;
        }
        fs::path streamPath = mArchivePath;
        if ( name != nullptr ) {
            streamPath = streamPath.parent_path();
            streamPath.append( name );
            std::error_code error;
            const auto streamStatus = fs::status( streamPath, error );
            if ( error || !fs::exists( streamStatus ) || fs::is_directory( streamStatus ) ) {
                return S_FALSE;
            }
        }

        try {
            auto inStreamTemp = bit7z::make_com< CFileInStream >( streamPath );
            *inStream = inStreamTemp.Detach();
        } catch ( const BitException& ex ) {
            return ex.nativeCode();
        }
        return S_OK;
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::SetSubArchiveName( const wchar_t* name ) noexcept {
    mSubArchiveMode = true;
    try {
        mSubArchiveName = name;
    } catch ( ... ) {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP OpenCallback::CryptoGetTextPassword( BSTR* password ) noexcept {
    mPasswordWasAsked = true;

    std::wstring pass;
    if ( !mHandler.isPasswordDefined() ) {
        if ( mHandler.passwordCallback() ) {
            pass = WIDEN( mHandler.passwordCallback()() );
        }

        if ( pass.empty() ) {
            return E_ABORT;
        }
    } else {
        pass = WIDEN( mHandler.password() );
    }

    return StringToBstr( pass.c_str(), password );
}

auto OpenCallback::passwordWasAsked() const -> bool {
    return mPasswordWasAsked;
}

} // namespace bit7z