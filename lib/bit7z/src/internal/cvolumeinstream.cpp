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

#include "internal/cvolumeinstream.hpp"

namespace bit7z {

CVolumeInStream::CVolumeInStream( const fs::path& volumePath, uint64_t globalOffset )
    : CFileInStream{ volumePath }, mSize{ fs::file_size( volumePath ) }, mGlobalOffset{ globalOffset } {}

BIT7Z_NODISCARD
auto CVolumeInStream::globalOffset() const -> uint64_t {
    return mGlobalOffset;
}

BIT7Z_NODISCARD
auto CVolumeInStream::size() const -> uint64_t {
    return mSize;
}

} // namespace bit7z