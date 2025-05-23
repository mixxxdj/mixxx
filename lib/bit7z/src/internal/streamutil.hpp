/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef STREAMUTIL_HPP
#define STREAMUTIL_HPP

#include <ios>

#include "internal/windows.hpp"

namespace bit7z {

inline auto to_seekdir( uint32_t seekOrigin, std::ios_base::seekdir& way ) -> HRESULT {
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            way = std::ios_base::beg;
            break;
        case STREAM_SEEK_CUR:
            way = std::ios_base::cur;
            break;
        case STREAM_SEEK_END:
            way = std::ios_base::end;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }
    return S_OK;
}

} // namespace bit7z

#endif //STREAMUTIL_HPP
