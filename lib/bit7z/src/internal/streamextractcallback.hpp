/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef STREAMEXTRACTCALLBACK_HPP
#define STREAMEXTRACTCALLBACK_HPP

#include <vector>
#include <map>

#include "internal/extractcallback.hpp"

namespace bit7z {

using std::ostream;

class StreamExtractCallback final : public ExtractCallback {
    public:
        StreamExtractCallback( const BitInputArchive& inputArchive, ostream& outputStream );

        StreamExtractCallback( const StreamExtractCallback& ) = delete;

        StreamExtractCallback( StreamExtractCallback&& ) = delete;

        auto operator=( const StreamExtractCallback& ) -> StreamExtractCallback& = delete;

        auto operator=( StreamExtractCallback&& ) -> StreamExtractCallback& = delete;

        ~StreamExtractCallback() override = default;

    private:
        ostream& mOutputStream;
        CMyComPtr< IOutStream > mStdOutStream;

        void releaseStream() override;

        auto getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT override;
};

}  // namespace bit7z

#endif // STREAMEXTRACTCALLBACK_HPP
