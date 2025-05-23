/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BUFFERITEM_HPP
#define BUFFERITEM_HPP

#include <string>

#include "internal/genericinputitem.hpp"

namespace bit7z {

using std::vector;

class BufferItem final : public GenericInputItem {
    public:
        explicit BufferItem( const vector< byte_t >& buffer, fs::path name );

        BIT7Z_NODISCARD auto name() const -> tstring override;

        BIT7Z_NODISCARD auto path() const -> tstring override;

        BIT7Z_NODISCARD auto inArchivePath() const -> fs::path override;

        BIT7Z_NODISCARD auto getStream( ISequentialInStream** inStream ) const -> HRESULT override;

        BIT7Z_NODISCARD auto isDir() const noexcept -> bool override;

        BIT7Z_NODISCARD auto size() const noexcept -> uint64_t override;

        BIT7Z_NODISCARD auto creationTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto lastAccessTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto lastWriteTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto attributes() const noexcept -> uint32_t override;

    private:
        const vector< byte_t >& mBuffer;
        fs::path mBufferName;
};

}  // namespace bit7z

#endif //BUFFERITEM_HPP
