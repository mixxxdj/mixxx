/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FAILURESOURCECATEGORY_HPP
#define FAILURESOURCECATEGORY_HPP

#include <system_error>

namespace bit7z {

struct FailureSourceCategory : public std::error_category {
    BIT7Z_NODISCARD
    auto name() const noexcept -> const char* override;

    BIT7Z_NODISCARD
    auto message( int errorValue ) const -> std::string override;

    BIT7Z_NODISCARD
    auto equivalent( int error, const std::error_condition& condition ) const noexcept -> bool override;

    BIT7Z_NODISCARD
    auto equivalent( const std::error_code& code, int condition ) const noexcept -> bool override;
};

auto source_category() noexcept -> const std::error_category&;

} // namespace bit7z

#endif //FAILURESOURCECATEGORY_HPP
