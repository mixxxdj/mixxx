/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FS_HPP
#define FS_HPP

#include "bitdefines.hpp"

#ifdef BIT7Z_USE_STANDARD_FILESYSTEM
#include <filesystem>
#include <fstream>

namespace bit7z {
namespace fs {
using namespace std::filesystem;
using ifstream = std::ifstream;
using ofstream = std::ofstream;
using fstream = std::fstream;
} // namespace fs
} // namespace bit7z
#else
#include <ghc/filesystem.hpp>

namespace bit7z {
namespace fs {
using namespace ghc::filesystem;
using ifstream = ghc::filesystem::ifstream;
using ofstream = ghc::filesystem::ofstream;
using fstream = ghc::filesystem::fstream;
} // namespace fs
} // namespace bit7z
#endif

#endif // FS_HPP
