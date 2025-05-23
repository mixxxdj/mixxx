/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CVOLUMEINSTREAM_HPP
#define CVOLUMEINSTREAM_HPP

#include "internal/cfileinstream.hpp"

namespace bit7z {

class CVolumeInStream final : public CFileInStream {
    public:
        explicit CVolumeInStream( const fs::path& volumePath, uint64_t globalOffset );

        BIT7Z_NODISCARD auto globalOffset() const -> uint64_t;

        BIT7Z_NODISCARD auto size() const -> uint64_t;

    private:
        uint64_t mSize;

        uint64_t mGlobalOffset;
};

}  // namespace bit7z

#endif //CVOLUMEINSTREAM_HPP
