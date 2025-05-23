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

#include "extractcallback.hpp"
#include "internal/operationcategory.hpp"

namespace bit7z {

auto OperationCategory::name() const noexcept -> const char* {
    return "operation";
}

auto OperationCategory::message( int errorValue ) const -> std::string {
    switch ( static_cast< OperationResult >( errorValue ) ) {
        case OperationResult::CRCError:
            return "CRC failed";
        case OperationResult::CRCErrorEncrypted:
            return "CRC error in encrypted file (wrong password?).";
        case OperationResult::DataAfterEnd:
            return "There are some data after the end of the payload data.";
        case OperationResult::DataError:
            return "Data error.";
        case OperationResult::DataErrorEncrypted:
            return "Data error in encrypted file (wrong password?).";
        case OperationResult::EmptyPassword:
            return "A password is required but none was provided.";
        case OperationResult::HeadersError:
            return "Headers error.";
        case OperationResult::IsNotArc:
            return "Invalid archive.";
        case OperationResult::OpenErrorEncrypted:
            return "Wrong password?";
        case OperationResult::WrongPassword:
            return "Wrong password.";
        case OperationResult::Unavailable:
            return "Unavailable data.";
        case OperationResult::UnexpectedEnd:
            return "Reached an unexpected end of data.";
        case OperationResult::UnsupportedMethod:
            return "Unsupported method.";
        default:
            return "Unknown error.";
    }
}

auto OperationCategory::default_error_condition( int errorValue ) const noexcept -> std::error_condition {
    switch ( static_cast< OperationResult >( errorValue ) ) {
        case OperationResult::UnsupportedMethod:
            return std::make_error_condition( std::errc::function_not_supported );
        case OperationResult::CRCError:
        case OperationResult::DataAfterEnd:
        case OperationResult::DataError:
        case OperationResult::HeadersError:
        case OperationResult::IsNotArc:
        case OperationResult::Unavailable:
        case OperationResult::UnexpectedEnd:
            return std::make_error_condition( std::errc::io_error );
        case OperationResult::WrongPassword:
        case OperationResult::EmptyPassword:
        case OperationResult::DataErrorEncrypted:
        case OperationResult::CRCErrorEncrypted:
        case OperationResult::OpenErrorEncrypted:
            return std::make_error_condition( std::errc::operation_not_permitted );
        default:
            return error_category::default_error_condition( errorValue );
    }
}

auto operation_category() noexcept -> const std::error_category& {
    static const OperationCategory instance{};
    return instance;
}

} // namespace bit7z