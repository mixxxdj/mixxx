/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PROCESSEDITEM_HPP
#define PROCESSEDITEM_HPP

#include "bitinputarchive.hpp"
#include "bitdefines.hpp"
#include "bitpropvariant.hpp"
#include "internal/fs.hpp"
#include "internal/windows.hpp"

namespace bit7z {

class ProcessedItem final {
    public:
        ProcessedItem();

        void loadItemInfo( const BitInputArchive& inputArchive, std::uint32_t itemIndex );

        BIT7Z_NODISCARD auto path() const -> fs::path;

        BIT7Z_NODISCARD auto attributes() const -> uint32_t;

        BIT7Z_NODISCARD auto areAttributesDefined() const -> bool;

        BIT7Z_NODISCARD auto hasModifiedTime() const -> bool;

        BIT7Z_NODISCARD auto modifiedTime() const -> FILETIME;

#ifdef _WIN32
        BIT7Z_NODISCARD auto hasCreationTime() const -> bool;

        BIT7Z_NODISCARD auto creationTime() const -> FILETIME;

        BIT7Z_NODISCARD auto hasAccessTime() const -> bool;

        BIT7Z_NODISCARD auto accessTime() const -> FILETIME;
#endif

    private:
        fs::path mFilePath;

        BitPropVariant mModifiedTime;
#ifdef _WIN32
        BitPropVariant mCreationTime;
        BitPropVariant mAccessTime;
#endif

        uint32_t mAttributes;
        bool mAreAttributesDefined;

        void loadFilePath( const BitInputArchive& inputArchive, uint32_t itemIndex );

        void loadAttributes( const BitInputArchive& inputArchive, uint32_t itemIndex );

        void loadTimeMetadata( const BitInputArchive& inputArchive, uint32_t itemIndex );
};

}  // namespace bit7z

#endif //PROCESSEDITEM_HPP
