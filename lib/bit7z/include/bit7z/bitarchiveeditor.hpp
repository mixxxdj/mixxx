/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITARCHIVEEDITOR_HPP
#define BITARCHIVEEDITOR_HPP

#include <unordered_map>

#include "bitarchivewriter.hpp"

namespace bit7z {

using std::vector;

using EditedItems = std::unordered_map< uint32_t, BitItemsVector::value_type >;

enum struct DeletePolicy : std::uint8_t {
    ItemOnly,
    RecurseDirs
};

/**
 * @brief The BitArchiveEditor class allows creating new file archives or updating old ones.
 *        Update operations supported are the addition of new items,
 *        as well as renaming/updating/deleting old items;
 *
 * @note  Changes are applied to the archive only after calling the applyChanges() method.
 */
class BIT7Z_MAYBE_UNUSED BitArchiveEditor final : public BitArchiveWriter {
    public:
        /**
         * @brief Constructs a BitArchiveEditor object, reading the given archive file path.
         *
         * @param lib      the 7z library to use.
         * @param inFile   the path to an input archive file.
         * @param format   the input/output archive format.
         * @param password (optional) the password needed to read the input archive.
         */
        BitArchiveEditor( const Bit7zLibrary& lib,
                          const tstring& inFile,
                          const BitInOutFormat& format,
                          const tstring& password = {} );

        BitArchiveEditor( const BitArchiveEditor& ) = delete;

        BitArchiveEditor( BitArchiveEditor&& ) = delete;

        auto operator=( const BitArchiveEditor& ) -> BitArchiveEditor& = delete;

        auto operator=( BitArchiveEditor&& ) -> BitArchiveEditor& = delete;

        ~BitArchiveEditor() override;

        /**
         * @brief Sets how the editor performs the update of the items in the archive.
         *
         * @note BitArchiveEditor doesn't support UpdateMode::None.
         *
         * @param mode the desired update mode (either UpdateMode::Append or UpdateMode::Overwrite).
         */
        void setUpdateMode( UpdateMode mode ) override;

        /**
         * @brief Requests to change the path of the item at the specified index with the given one.
         *
         * @param index    the index of the item to be renamed.
         * @param newPath the new path (in the archive) desired for the item.
         */
        void renameItem( uint32_t index, const tstring& newPath );

        /**
         * @brief Requests to change the path of the item from oldPath to the newPath.
         *
         * @param oldPath the old path (in the archive) of the item to be renamed.
         * @param newPath the new path (in the archive) desired for the item.
         */
        void renameItem( const tstring& oldPath, const tstring& newPath );

        /**
         * @brief Requests to update the content of the item at the specified index
         *        with the data from the given file.
         *
         * @param index     the index of the item to be updated.
         * @param inFile    the path to the file containing the new data for the item.
         */
        void updateItem( uint32_t index, const tstring& inFile );

        /**
         * @brief Requests to update the content of the item at the specified index
         *        with the data from the given buffer.
         *
         * @param index     the index of the item to be updated.
         * @param inBuffer  the buffer containing the new data for the item.
         */
        void updateItem( uint32_t index, const std::vector< byte_t >& inBuffer );

        /**
         * @brief Requests to update the content of the item at the specified index
         *        with the data from the given stream.
         *
         * @param index     the index of the item to be updated.
         * @param inStream  the stream of new data for the item.
         */
        void updateItem( uint32_t index, std::istream& inStream );

        /**
         * @brief Requests to update the content of the item at the specified path
         *        with the data from the given file.
         *
         * @param itemPath  the path (in the archive) of the item to be updated.
         * @param inFile    the path to the file containing the new data for the item.
         */
        void updateItem( const tstring& itemPath, const tstring& inFile );

        /**
         * @brief Requests to update the content of the item at the specified path
         *        with the data from the given buffer.
         *
         * @param itemPath  the path (in the archive) of the item to be updated.
         * @param inBuffer  the buffer containing the new data for the item.
         */
        void updateItem( const tstring& itemPath, const std::vector< byte_t >& inBuffer );

        /**
         * @brief Requests to update the content of the item at the specified path
         *        with the data from the given stream.
         *
         * @param itemPath  the path (in the archive) of the item to be updated.
         * @param inStream  the stream of new data for the item.
         */
        void updateItem( const tstring& itemPath, istream& inStream );

        /**
         * @brief Marks as deleted the item at the given index.
         *
         * @note By default, if the item is a folder, only its metadata is deleted, not the files within it.
         *       If instead the policy is set to DeletePolicy::RecurseDirs,
         *       then the items within the folder will also be deleted.
         *
         * @param index  the index of the item to be deleted.
         * @param policy the policy to be used when deleting items.
         *
         * @throws BitException if the index is invalid.
         */
        void deleteItem( uint32_t index, DeletePolicy policy = DeletePolicy::ItemOnly );

        /**
         * @brief Marks as deleted the archive's item(s) with the specified path.
         *
         * @note By default, if the marked item is a folder, only its metadata will be deleted, not the files within it.
         *       To delete the folder contents as well, set the `policy` to `DeletePolicy::RecurseDirs`.
         *
         * @note The specified path must not begin with a path separator.
         *
         * @note A path with a trailing separator will _only_ be considered if
         *       the policy is DeletePolicy::RecurseDirs, and will only match folders;
         *       with DeletePolicy::ItemOnly, no item will match a path with a trailing separator.
         *
         * @note Generally, archives may contain multiple items with the same paths.
         *       If this is the case, all matching items will be marked as deleted according to the specified policy.
         *
         * @param itemPath the path (in the archive) of the item to be deleted.
         * @param policy   the policy to be used when deleting items.
         *
         * @throws BitException if the specified path is empty or invalid, or if no matching item could be found.
         */
        void deleteItem( const tstring& itemPath, DeletePolicy policy = DeletePolicy::ItemOnly );

        /**
         * @brief Applies the requested changes (i.e., rename/update/delete operations) to the input archive.
         */
        void applyChanges();

    private:
        EditedItems mEditedItems;

        auto findItem( const tstring& itemPath ) -> uint32_t;

        void checkIndex( uint32_t index );

        auto itemProperty( InputIndex index, BitProperty property ) const -> BitPropVariant override;

        auto itemStream( InputIndex index, ISequentialInStream** inStream ) const -> HRESULT override;

        auto hasNewData( uint32_t index ) const noexcept -> bool override;

        auto hasNewProperties( uint32_t index ) const noexcept -> bool override;

        void markItemAsDeleted( uint32_t index );
};

}  // namespace bit7z

#endif //BITARCHIVEEDITOR_HPP
