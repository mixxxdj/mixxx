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

#include "bittypes.hpp"

#if defined( _WIN32 ) && !defined( BIT7Z_USE_NATIVE_STRING )
#include "internal/stringutil.hpp"
#endif

namespace bit7z {

#if defined( _WIN32 ) && !defined( BIT7Z_USE_NATIVE_STRING )
auto to_tstring( const native_string& str ) -> tstring {
    return narrow( str.c_str(), str.size() );
}
#else
auto to_tstring( const native_string& str ) -> const tstring& {
    return str;
}
#endif

} // namespace bit7z