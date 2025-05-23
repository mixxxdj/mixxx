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

#include "internal/genericinputitem.hpp"
#include "internal/stringutil.hpp"

namespace bit7z {

auto GenericInputItem::hasNewData() const noexcept -> bool {
    return true;
}

auto GenericInputItem::itemProperty( BitProperty property ) const -> BitPropVariant {
    BitPropVariant prop;
    switch ( property ) {
        case BitProperty::Path:
            prop = path_to_wide_string( inArchivePath() );
            break;
        case BitProperty::IsDir:
            prop = isDir();
            break;
        case BitProperty::Size:
            prop = size();
            break;
        case BitProperty::Attrib:
            prop = attributes();
            break;
        case BitProperty::CTime:
            prop = creationTime();
            break;
        case BitProperty::ATime:
            prop = lastAccessTime();
            break;
        case BitProperty::MTime:
            prop = lastWriteTime();
            break;
        default: //empty prop
            break;
    }
    return prop;
}

auto GenericInputItem::isSymLink() const -> bool {
    return false;
}

} // namespace bit7z