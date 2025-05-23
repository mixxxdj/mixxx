/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FSITEM_HPP
#define FSITEM_HPP

#include "internal/fsutil.hpp"
#include "internal/genericinputitem.hpp"
#include "internal/windows.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

class FilesystemItem final : public GenericInputItem {
    public:
        explicit FilesystemItem( const fs::path& itemPath,
                                 fs::path inArchivePath = fs::path{},
                                 SymlinkPolicy symlinkPolicy = SymlinkPolicy::Follow );

        explicit FilesystemItem( fs::directory_entry entry,
                                 const fs::path& searchPath,
                                 SymlinkPolicy symlinkPolicy );

        BIT7Z_NODISCARD auto isDots() const -> bool;

        BIT7Z_NODISCARD auto isDir() const noexcept -> bool override;

        BIT7Z_NODISCARD auto isSymLink() const -> bool override;

        BIT7Z_NODISCARD auto size() const noexcept -> uint64_t override;

        BIT7Z_NODISCARD auto creationTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto lastAccessTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto lastWriteTime() const noexcept -> FILETIME override;

        BIT7Z_NODISCARD auto name() const -> tstring override;

        BIT7Z_NODISCARD auto path() const -> tstring override;

        BIT7Z_NODISCARD auto inArchivePath() const -> fs::path override;

        BIT7Z_NODISCARD auto attributes() const noexcept -> uint32_t override;

        BIT7Z_NODISCARD auto itemProperty( BitProperty property ) const -> BitPropVariant override;

        BIT7Z_NODISCARD auto getStream( ISequentialInStream** inStream ) const -> HRESULT override;

        BIT7Z_NODISCARD auto filesystemPath() const -> const fs::path&;

        BIT7Z_NODISCARD auto filesystemName() const -> fs::path;


    private:
        fs::directory_entry mFileEntry;
        WIN32_FILE_ATTRIBUTE_DATA mFileAttributeData;
        fs::path mInArchivePath;
        SymlinkPolicy mSymlinkPolicy;

        void initAttributes( const fs::path& itemPath );
};

}  // namespace filesystem
}  // namespace bit7z

#endif // FSITEM_HPP
