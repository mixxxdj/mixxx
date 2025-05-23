/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef HRESULTCATEGORY_HPP
#define HRESULTCATEGORY_HPP

#include <system_error>
#include <string>

#include "bitdefines.hpp"
#include "bitwindows.hpp"

namespace bit7z {

struct HRESULTCategory final : public std::error_category {
    static_assert( sizeof( int ) >= sizeof( HRESULT ), "HRESULT type must be at least the size of int" );

    explicit HRESULTCategory() = default;

    BIT7Z_NODISCARD auto name() const noexcept -> const char* override;

    BIT7Z_NODISCARD auto message( int errorValue ) const -> std::string override;

    BIT7Z_NODISCARD auto default_error_condition( int errorValue ) const noexcept -> std::error_condition override;
};

auto hresult_category() noexcept -> const std::error_category&;

}  // namespace bit7z

#endif //HRESULTCATEGORY_HPP
