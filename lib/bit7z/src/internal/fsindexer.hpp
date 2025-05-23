/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FSINDEXER_HPP
#define FSINDEXER_HPP

#include <string>
#include <vector>
#include <map>

#include "bitabstractarchivehandler.hpp"
#include "internal/fsitem.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace filesystem {

using std::vector;
using std::unique_ptr;

class FilesystemIndexer final {
    public:
        explicit FilesystemIndexer( FilesystemItem directory,
                                    tstring filter = {},
                                    FilterPolicy policy = FilterPolicy::Include,
                                    SymlinkPolicy symlinkPolicy = SymlinkPolicy::Follow,
                                    bool onlyFiles = false );

        void listDirectoryItems( std::vector< std::unique_ptr< GenericInputItem > >& result, bool recursive );

    private:
        FilesystemItem mDirItem;
        tstring mFilter;
        FilterPolicy mPolicy;
        SymlinkPolicy mSymlinkPolicy;
        bool mOnlyFiles;
};

}  // namespace filesystem
}  // namespace bit7z

#endif // FSINDEXER_HPP
