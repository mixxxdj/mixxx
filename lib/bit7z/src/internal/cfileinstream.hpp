/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CFILEINSTREAM_HPP
#define CFILEINSTREAM_HPP

#include <array>

#include "bitdefines.hpp"
#include "internal/cstdinstream.hpp"
#include "internal/fs.hpp"

namespace bit7z {

class CFileInStream : public CStdInStream {
    public:
        explicit CFileInStream( const fs::path& filePath );

        void openFile( const fs::path& filePath );

    private:
        fs::ifstream mFileStream;
};

}  // namespace bit7z

#endif // CFILEINSTREAM_HPP
