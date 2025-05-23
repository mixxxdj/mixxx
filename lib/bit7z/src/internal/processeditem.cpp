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

#include "bitexception.hpp"
#include "internal/processeditem.hpp"

namespace bit7z {

ProcessedItem::ProcessedItem()
    : mAttributes{ 0 }, mAreAttributesDefined{ false } {}

void ProcessedItem::loadItemInfo( const BitInputArchive& inputArchive, std::uint32_t itemIndex ) {
    loadFilePath( inputArchive, itemIndex );
    loadAttributes( inputArchive, itemIndex );
    loadTimeMetadata( inputArchive, itemIndex );
}

auto ProcessedItem::path() const -> fs::path {
    return mFilePath;
}

auto ProcessedItem::attributes() const -> uint32_t {
    return mAttributes;
}

auto ProcessedItem::hasModifiedTime() const -> bool {
    return mModifiedTime.isFileTime();
}

auto ProcessedItem::modifiedTime() const -> FILETIME {
    return mModifiedTime.getFileTime();
}

#ifdef _WIN32
auto ProcessedItem::hasCreationTime() const -> bool {
    return mCreationTime.isFileTime();
}

auto ProcessedItem::creationTime() const -> FILETIME {
    return mCreationTime.getFileTime();
}

auto ProcessedItem::hasAccessTime() const -> bool {
    return mAccessTime.isFileTime();
}

auto ProcessedItem::accessTime() const -> FILETIME {
    return mAccessTime.getFileTime();
}
#endif

void ProcessedItem::loadFilePath( const BitInputArchive& inputArchive, uint32_t itemIndex ) {
    const BitPropVariant prop = inputArchive.itemProperty( itemIndex, BitProperty::Path );

    switch ( prop.type() ) {
        case BitPropVariantType::Empty:
            mFilePath = fs::path{};
            break;

        case BitPropVariantType::String:
            mFilePath = fs::path{ prop.getNativeString() };
            break;

        default:
            throw BitException( "Could not load file path information of item", make_hresult_code( E_FAIL ) );
    }
}

void ProcessedItem::loadAttributes( const BitInputArchive& inputArchive, uint32_t itemIndex ) {
    mAttributes = 0;
    mAreAttributesDefined = false;

    // Get posix attributes
    const BitPropVariant posixAttributes = inputArchive.itemProperty( itemIndex, BitProperty::PosixAttrib );
    switch ( posixAttributes.type() ) {
        case BitPropVariantType::Empty:
            break;

        case BitPropVariantType::UInt32:
            mAttributes = ( posixAttributes.getUInt32() << 16u ) | FILE_ATTRIBUTE_UNIX_EXTENSION;
            mAreAttributesDefined = true;
            break;

        default:
            throw BitException( "Could not load posix attributes of item", make_hresult_code( E_FAIL ) );
    }

    // Get attributes
    const BitPropVariant attributes = inputArchive.itemProperty( itemIndex, BitProperty::Attrib );
    switch ( attributes.type() ) {
        case BitPropVariantType::Empty:
            break;

        case BitPropVariantType::UInt32:
            mAttributes = attributes.getUInt32();
            mAreAttributesDefined = true;
            break;

        default:
            throw BitException( "Could not load attributes of item", make_hresult_code( E_FAIL ) );
    }
}

void ProcessedItem::loadTimeMetadata( const BitInputArchive& inputArchive, uint32_t itemIndex ) {
    mModifiedTime = inputArchive.itemProperty( itemIndex, BitProperty::MTime );
#ifdef _WIN32
    mCreationTime = inputArchive.itemProperty( itemIndex, BitProperty::CTime );
    mAccessTime = inputArchive.itemProperty( itemIndex, BitProperty::ATime );
#endif
}

auto ProcessedItem::areAttributesDefined() const -> bool {
    return mAreAttributesDefined;
}

} // namespace bit7z