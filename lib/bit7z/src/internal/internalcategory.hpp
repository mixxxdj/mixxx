/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef INTERNALCATEGORY_HPP
#define INTERNALCATEGORY_HPP

#include <system_error>
#include <string>

#include "bitdefines.hpp"

namespace bit7z {

struct InternalCategory final : public std::error_category {
    BIT7Z_NODISCARD auto name() const noexcept -> const char* override;

    BIT7Z_NODISCARD auto message( int errorValue ) const -> std::string override;

    BIT7Z_NODISCARD auto default_error_condition( int errorValue ) const noexcept -> std::error_condition override;
};

auto internal_category() noexcept -> const std::error_category&;

}  // namespace bit7z

#endif //INTERNALCATEGORY_HPP
