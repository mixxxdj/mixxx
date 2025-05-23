/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef STDINPUTITEM_HPP
#define STDINPUTITEM_HPP

#include "internal/genericinputitem.hpp"

namespace bit7z {

using std::istream;

class StdInputItem final : public GenericInputItem {
    public:
        explicit StdInputItem( istream& stream, fs::path path );

        BIT7Z_NODISCARD auto name() const -> tstring override;

        BIT7Z_NODISCARD auto isDir() const noexcept -> bool override;

        BIT7Z_NODISCARD auto size() const -> uint64_t override;

        BIT7Z_NODISCARD auto creationTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto lastAccessTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto lastWriteTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto attributes() const noexcept -> uint32_t override;

        BIT7Z_NODISCARD auto path() const -> tstring override;

        BIT7Z_NODISCARD auto inArchivePath() const -> fs::path override;

        BIT7Z_NODISCARD auto getStream( ISequentialInStream** inStream ) const -> HRESULT override;

    private:
        istream& mStream;
        fs::path mStreamPath;
};

}  // namespace bit7z

#endif //STDINPUTITEM_HPP
