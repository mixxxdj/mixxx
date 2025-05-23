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

#include "internal/bufferutil.hpp"
#include "internal/windows.hpp"

auto bit7z::seek( const buffer_t& buffer,
                  const buffer_t::const_iterator& currentPosition,
                  int64_t offset,
                  uint32_t seekOrigin,
                  uint64_t& newPosition ) -> HRESULT {
    uint64_t currentIndex{};
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET: {
            break;
        }
        case STREAM_SEEK_CUR: {
            currentIndex = static_cast< uint64_t >( currentPosition - buffer.cbegin() );
            break;
        }
        case STREAM_SEEK_END: {
            currentIndex = static_cast< uint64_t >( buffer.cend() - buffer.cbegin() );
            break;
        }
        default:
            return STG_E_INVALIDFUNCTION;
    }

    RINOK( seek_to_offset( currentIndex, offset ) )

    if ( currentIndex > buffer.size() ) {
        return E_INVALIDARG;
    }

    newPosition = currentIndex;
    return S_OK;
}