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

#include "internal/cfixedbufferoutstream.hpp"
#include "internal/fixedbufferextractcallback.hpp"
#include "internal/util.hpp"

namespace bit7z {

FixedBufferExtractCallback::FixedBufferExtractCallback( const BitInputArchive& inputArchive,
                                                        byte_t* buffer,
                                                        size_t size )
    : ExtractCallback( inputArchive ), mBuffer( buffer ), mSize( size ) {}

void FixedBufferExtractCallback::releaseStream() {
    mOutMemStream.Release();
}

auto FixedBufferExtractCallback::getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT {
    if ( isItemFolder( index ) ) {
        return S_OK;
    }

    // Get Name
    const BitPropVariant prop = itemProperty( index, BitProperty::Path );
    tstring fullPath;

    if ( prop.isEmpty() ) {
        fullPath = kEmptyFileAlias;
    } else if ( prop.isString() ) {
        fullPath = prop.getString();
    } else {
        return E_FAIL;
    }

    if ( mHandler.fileCallback() ) {
        mHandler.fileCallback()( fullPath );
    }

    auto outStreamLoc = bit7z::make_com< CFixedBufferOutStream, ISequentialOutStream >( mBuffer, mSize );
    mOutMemStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
    return S_OK;
}

} // namespace bit7z