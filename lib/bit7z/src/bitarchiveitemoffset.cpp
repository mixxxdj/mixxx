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

#include "bitarchiveitemoffset.hpp"

#include "bitinputarchive.hpp"

using namespace bit7z;

BitArchiveItemOffset::BitArchiveItemOffset( uint32_t itemIndex, const BitInputArchive& inputArchive ) noexcept
    : BitArchiveItem( itemIndex ), mArc( &inputArchive ) {}

auto BitArchiveItemOffset::operator++() noexcept -> BitArchiveItemOffset& {
    ++mItemIndex;
    return *this;
}

auto BitArchiveItemOffset::operator++( int ) noexcept -> BitArchiveItemOffset { // NOLINT(cert-dcl21-cpp)
    BitArchiveItemOffset oldValue = *this;
    ++( *this );
    return oldValue;
}

auto BitArchiveItemOffset::operator==( const BitArchiveItemOffset& other ) const noexcept -> bool {
    return mItemIndex == other.mItemIndex;
}

auto BitArchiveItemOffset::operator!=( const BitArchiveItemOffset& other ) const noexcept -> bool {
    return !( *this == other );
}

auto BitArchiveItemOffset::itemProperty( BitProperty property ) const -> BitPropVariant {
    return mArc != nullptr ? mArc->itemProperty( mItemIndex, property ) : BitPropVariant();
}
