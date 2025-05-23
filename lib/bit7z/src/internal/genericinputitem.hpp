/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GENERICINPUTITEM_HPP
#define GENERICINPUTITEM_HPP

#include <cstdint>

#include "bitgenericitem.hpp"
#include "internal/fs.hpp"

struct ISequentialInStream;

namespace bit7z {

struct GenericInputItem : public BitGenericItem {
    BIT7Z_NODISCARD auto isSymLink() const -> bool override;

    BIT7Z_NODISCARD virtual auto inArchivePath() const -> fs::path = 0;

    BIT7Z_NODISCARD virtual auto getStream( ISequentialInStream** inStream ) const -> HRESULT = 0;

    BIT7Z_NODISCARD virtual auto creationTime() const -> FILETIME = 0;

    BIT7Z_NODISCARD virtual auto lastAccessTime() const -> FILETIME = 0;

    BIT7Z_NODISCARD virtual auto lastWriteTime() const -> FILETIME = 0;

    BIT7Z_NODISCARD virtual auto hasNewData() const noexcept -> bool;

    BIT7Z_NODISCARD auto itemProperty( BitProperty property ) const -> BitPropVariant override;

    ~GenericInputItem() override = default;
};

}  // namespace bit7z

#endif //GENERICINPUTITEM_HPP
