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
#include "internal/internalcategory.hpp"

namespace bit7z {

auto make_error_code( BitError error ) -> std::error_code {
    return { static_cast< int >( error ), internal_category() };
}

auto make_error_condition( BitFailureSource failureSource ) -> std::error_condition {
    return { static_cast< int >( failureSource ), source_category() };
}

} // namespace bit7z