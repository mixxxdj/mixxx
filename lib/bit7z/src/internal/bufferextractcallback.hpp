/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BUFFEREXTRACTCALLBACK_HPP
#define BUFFEREXTRACTCALLBACK_HPP

#include <vector>
#include <map>

#include "internal/extractcallback.hpp"

namespace bit7z {

using std::vector;
using std::map;

class BufferExtractCallback final : public ExtractCallback {
    public:
        BufferExtractCallback( const BitInputArchive& inputArchive,
                               map< tstring, vector< byte_t > >& buffersMap );

        BufferExtractCallback( const BufferExtractCallback& ) = delete;

        BufferExtractCallback( BufferExtractCallback&& ) = delete;

        auto operator=( const BufferExtractCallback& ) -> BufferExtractCallback& = delete;

        auto operator=( BufferExtractCallback&& ) -> BufferExtractCallback& = delete;

        ~BufferExtractCallback() override = default;

    private:
        map< tstring, vector< byte_t > >& mBuffersMap;
        CMyComPtr< ISequentialOutStream > mOutMemStream;

        void releaseStream() override;

        auto getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT override;
};

}  // namespace bit7z
#endif // BUFFEREXTRACTCALLBACK_HPP
