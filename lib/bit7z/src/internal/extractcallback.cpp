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

#include <exception>

#include "bitexception.hpp"
#include "internal/extractcallback.hpp"
#include "internal/operationcategory.hpp"
#include "internal/stringutil.hpp"

namespace bit7z {

ExtractCallback::ExtractCallback( const BitInputArchive& inputArchive )
    : Callback( inputArchive.handler() ),
      mInputArchive( inputArchive ),
      mExtractMode( ExtractMode::Extract ),
      mIsLastItemEncrypted{ false } {}

auto ExtractCallback::finishOperation( OperationResult operationResult ) -> HRESULT {
    releaseStream();
    return operationResult != OperationResult::Success ? E_FAIL : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetTotal( UInt64 size ) noexcept {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetCompleted( const UInt64* completeValue ) noexcept {
    if ( mHandler.progressCallback() && completeValue != nullptr ) {
        return mHandler.progressCallback()( *completeValue ) ? S_OK : E_ABORT;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) noexcept {
    if ( mHandler.ratioCallback() && inSize != nullptr && outSize != nullptr ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::PrepareOperation( Int32 askExtractMode ) noexcept {
    // in the future, we might use a switch to handle an event like onOperationStart(Operation o)
    // with enum ExtractMode.
    mExtractMode = static_cast< ExtractMode >( askExtractMode );
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode ) noexcept
try {
    *outStream = nullptr;
    releaseStream();

    auto isEncrypted = itemProperty( index, BitProperty::Encrypted );
    if ( isEncrypted.isBool() ) {
        mIsLastItemEncrypted = isEncrypted.getBool();
    }

    if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract ) {
        return S_OK;
    }

    return getOutStream( index, outStream );
} catch ( const BitException& ex ) {
    mErrorException = std::make_exception_ptr( ex );
    return ex.hresultCode();
} catch ( const std::runtime_error& ) {
    mErrorException = std::make_exception_ptr(
        BitException( "Failed to get the stream", make_hresult_code( E_ABORT ) ) );
    return E_ABORT;
}

auto map_operation_result( Int32 operationResult, bool isLastItemEncrypted ) -> OperationResult {
    if ( isLastItemEncrypted ) {
        if ( operationResult == NOperationResult::kCRCError ) {
            return OperationResult::CRCErrorEncrypted;
        }

        if ( operationResult == NOperationResult::kDataError ) {
            return OperationResult::DataErrorEncrypted;
        }
    }

    return static_cast< OperationResult >( operationResult );
}

constexpr auto kTestFailed = "Failed to test the archive";
constexpr auto kExtractFailed = "Failed to extract the archive";

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::SetOperationResult( Int32 operationResult ) noexcept {
    using namespace NArchive::NExtract;

    auto result = map_operation_result( operationResult, mIsLastItemEncrypted );
    if ( result != OperationResult::Success ) {
        const auto* msg = mExtractMode == ExtractMode::Test ? kTestFailed : kExtractFailed;
        auto error = make_error_code( result );
        mErrorException = std::make_exception_ptr( BitException( msg, error ) );
    }

    return finishOperation( result );
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP ExtractCallback::CryptoGetTextPassword( BSTR* password ) noexcept {
    std::wstring pass;
    if ( !mHandler.isPasswordDefined() ) {
        if ( mHandler.passwordCallback() ) {
            pass = WIDEN( mHandler.passwordCallback()() );
        }

        if ( pass.empty() ) {
            const auto* msg = mExtractMode == ExtractMode::Test ? kTestFailed : kExtractFailed;
            auto error = make_error_code( OperationResult::EmptyPassword );
            mErrorException = std::make_exception_ptr( BitException( msg, error ) );
            return E_FAIL;
        }
    } else {
        pass = WIDEN( mHandler.password() );
    }

    return StringToBstr( pass.c_str(), password );
}

} // namespace bit7z
