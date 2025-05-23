/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RENAMEDITEM_HPP
#define RENAMEDITEM_HPP

#include "bitinputarchive.hpp"
#include "internal/genericinputitem.hpp"

namespace bit7z {

class RenamedItem final : public GenericInputItem {
    public:
        explicit RenamedItem( const BitInputArchive& inputArchive, uint32_t index, const tstring& newPath );

        BIT7Z_NODISCARD auto name() const -> tstring override;

        BIT7Z_NODISCARD auto isDir() const -> bool override;

        BIT7Z_NODISCARD auto isSymLink() const -> bool override;

        BIT7Z_NODISCARD auto size() const -> uint64_t override;

        BIT7Z_NODISCARD auto creationTime() const -> FILETIME override;

        BIT7Z_NODISCARD auto lastAccessTime() const -> FILETIME override;

        BIT7Z_NODISCARD auto lastWriteTime() const -> FILETIME override;

        BIT7Z_NODISCARD auto attributes() const -> uint32_t override;

        BIT7Z_NODISCARD auto path() const -> tstring override;

        BIT7Z_NODISCARD auto inArchivePath() const -> fs::path override;

        BIT7Z_NODISCARD auto getStream( ISequentialInStream** inStream ) const noexcept -> HRESULT override;

        BIT7Z_NODISCARD auto hasNewData() const noexcept -> bool override;

    private:
        const BitInputArchive& mInputArchive;
        uint32_t mIndex;
        fs::path mNewPath;
};

}  // namespace bit7z

#endif //RENAMEDITEM_HPP
