/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GUIDS_HPP
#define GUIDS_HPP

#include "internal/guiddef.hpp"

namespace bit7z {

extern "C" {
#ifndef _WIN32
extern const GUID IID_IUnknown;
#endif

// IStream.h
extern const GUID IID_ISequentialInStream;
extern const GUID IID_ISequentialOutStream;
extern const GUID IID_IInStream;
extern const GUID IID_IOutStream;
extern const GUID IID_IStreamGetSize;
extern const GUID IID_IStreamGetProps;
extern const GUID IID_IStreamGetProps2;

// ICoder.h
extern const GUID IID_ICompressProgressInfo;

// IPassword.h
extern const GUID IID_ICryptoGetTextPassword;
extern const GUID IID_ICryptoGetTextPassword2;

// IArchive.h
extern const GUID IID_ISetProperties;
extern const GUID IID_IInArchive;
extern const GUID IID_IOutArchive;
extern const GUID IID_IArchiveExtractCallback;
extern const GUID IID_IArchiveOpenVolumeCallback;
extern const GUID IID_IArchiveOpenSetSubArchiveName;
extern const GUID IID_IArchiveUpdateCallback;
extern const GUID IID_IArchiveUpdateCallback2;
}

}  // namespace bit7z

#endif // GUIDS_HPP
