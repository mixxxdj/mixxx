/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DATEUTIL_HPP
#define DATEUTIL_HPP

#include <chrono>
#include <ctime>

#include "bitgenericitem.hpp"
#include "bitwindows.hpp"
#include "internal/fs.hpp"

namespace bit7z {

#ifndef _WIN32

auto FILETIME_to_file_time_type( FILETIME fileTime ) -> fs::file_time_type;

auto time_to_FILETIME( std::time_t value ) -> FILETIME;

#endif

auto FILETIME_to_time_type( FILETIME fileTime ) -> time_type;

auto current_file_time() -> FILETIME;

}  // namespace bit7z

#endif //DATEUTIL_HPP
