/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef OPERATIONRESULT_HPP
#define OPERATIONRESULT_HPP

#include <system_error>
#include <type_traits>

#include "internal/com.hpp" // To be included before IArchive.h, since on Unix it redefines some of 7-zip's macros.

#include <7zip/Archive/IArchive.h>

using namespace NArchive::NExtract;

namespace bit7z {

enum struct OperationResult {
    Success = NOperationResult::kOK,
    UnsupportedMethod = NOperationResult::kUnsupportedMethod,
    DataError = NOperationResult::kDataError,
    CRCError = NOperationResult::kCRCError,
    Unavailable = NOperationResult::kUnavailable,
    UnexpectedEnd = NOperationResult::kUnexpectedEnd,
    DataAfterEnd = NOperationResult::kDataAfterEnd,
    IsNotArc = NOperationResult::kIsNotArc,
    HeadersError = NOperationResult::kHeadersError,
    WrongPassword = NOperationResult::kWrongPassword,
    DataErrorEncrypted = 2 * NOperationResult::kWrongPassword,
    CRCErrorEncrypted = ( 2 * NOperationResult::kWrongPassword ) + 1,
    OpenErrorEncrypted = ( 2 * NOperationResult::kWrongPassword ) + 2,
    EmptyPassword = ( 2 * NOperationResult::kWrongPassword ) + 3,
};

auto make_error_code( OperationResult error ) -> std::error_code;

} // namespace bit7z

namespace std {
template <>
struct BIT7Z_MAYBE_UNUSED is_error_code_enum< bit7z::OperationResult > : public true_type {};
} // namespace std

#endif // OPERATIONRESULT_HPP
