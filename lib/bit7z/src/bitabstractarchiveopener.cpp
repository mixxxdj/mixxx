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

#include "bitabstractarchiveopener.hpp"

using namespace bit7z;

BitAbstractArchiveOpener::BitAbstractArchiveOpener( const Bit7zLibrary& lib,
                                                    const BitInFormat& format,
                                                    const tstring& password )
    : BitAbstractArchiveHandler{ lib, password, OverwriteMode::Overwrite }, mFormat{ format } {}

auto BitAbstractArchiveOpener::format() const noexcept -> const BitInFormat& {
    return mFormat;
}

auto BitAbstractArchiveOpener::extractionFormat() const noexcept -> const BitInFormat& {
    return mFormat;
}
