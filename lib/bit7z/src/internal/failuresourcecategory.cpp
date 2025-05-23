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
#include "internal/failuresourcecategory.hpp"
#include "internal/operationcategory.hpp"
#include "internal/operationresult.hpp"

namespace bit7z {

auto FailureSourceCategory::name() const noexcept -> const char* {
    return "failure-source";
}

auto FailureSourceCategory::message( int errorValue ) const -> std::string {
    switch ( static_cast< BitFailureSource >( errorValue ) ) {
        case BitFailureSource::CRCError:
            return operation_category().message( NOperationResult::kCRCError );
        case BitFailureSource::DataAfterEnd:
            return operation_category().message( NOperationResult::kDataAfterEnd );
        case BitFailureSource::DataError:
            return operation_category().message( NOperationResult::kDataError );
        case BitFailureSource::FormatDetectionError:
            return "Format detection error.";
        case BitFailureSource::HeadersError:
            return operation_category().message( NOperationResult::kHeadersError );
        case BitFailureSource::InvalidArchive:
            return operation_category().message( NOperationResult::kIsNotArc );
        case BitFailureSource::InvalidArgument:
            return "Invalid argument.";
        case BitFailureSource::NoSuchItem:
            return "No such item.";
        case BitFailureSource::OperationNotPermitted:
            return "Operation not permitted.";
        case BitFailureSource::OperationNotSupported:
            return "Operation not supported.";
        case BitFailureSource::UnavailableData:
            return operation_category().message( NOperationResult::kUnavailable );
        case BitFailureSource::UnexpectedEnd:
            return operation_category().message( NOperationResult::kUnexpectedEnd );
        case BitFailureSource::WrongPassword:
            return "Wrong password.";
        default:
            return "Unknown failure source.";
    }
}

auto FailureSourceCategory::equivalent( int error, const std::error_condition& condition ) const noexcept -> bool {
    return default_error_condition( error ) == condition;
}

auto FailureSourceCategory::equivalent( const std::error_code& code, int condition ) const noexcept -> bool {
    switch ( static_cast< BitFailureSource >( condition ) ) {
        case BitFailureSource::CRCError:
            return code == OperationResult::CRCError || code == OperationResult::CRCErrorEncrypted;
        case BitFailureSource::DataAfterEnd:
            return code == OperationResult::DataAfterEnd;
        case BitFailureSource::DataError:
            return code == OperationResult::DataError || code == OperationResult::DataErrorEncrypted;
        case BitFailureSource::InvalidArchive:
            return code == OperationResult::IsNotArc || code == BitError::NoMatchingSignature;
        case BitFailureSource::InvalidArgument:
            return code == std::errc::invalid_argument;
        case BitFailureSource::HeadersError:
            return code == OperationResult::HeadersError;
        case BitFailureSource::FormatDetectionError:
            return code == BitError::NoMatchingSignature;
        case BitFailureSource::NoSuchItem:
            return code == BitError::NoMatchingItems || code == std::errc::no_such_file_or_directory;
        case BitFailureSource::OperationNotSupported:
            return code == std::errc::operation_not_supported ||
                   code == std::errc::not_supported ||
                   code == std::errc::function_not_supported;
        case BitFailureSource::OperationNotPermitted:
            return code == std::errc::operation_not_permitted;
        case BitFailureSource::UnavailableData:
            return code == OperationResult::Unavailable;
        case BitFailureSource::UnexpectedEnd:
            return code == OperationResult::UnexpectedEnd;
        case BitFailureSource::WrongPassword:
            return code == OperationResult::WrongPassword ||
                   code == OperationResult::DataErrorEncrypted ||
                   code == OperationResult::CRCErrorEncrypted ||
                   code == OperationResult::OpenErrorEncrypted ||
                   code == OperationResult::EmptyPassword;
        default:
            return false;
    }
}

auto source_category() noexcept -> const std::error_category& {
    static const FailureSourceCategory instance{};
    return instance;
}

} // namespace bit7z