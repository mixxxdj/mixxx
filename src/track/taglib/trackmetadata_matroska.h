#pragma once

// TagLib < 2.2 has no Matroska module; the header only exists from 2.2 on.
// The TAGLIB_* macros are defined by taglib_config.h, pulled in via the
// taglib headers included through trackmetadata.h before this file.
#if (TAGLIB_MAJOR_VERSION > 2) || \
        ((TAGLIB_MAJOR_VERSION == 2) && (TAGLIB_MINOR_VERSION >= 2))
#include <matroskafile.h>

#include "track/taglib/trackmetadata_file.h"

namespace mixxx {

namespace taglib {

namespace matroska {

bool importCoverImageFromTag(
        QImage* pCoverArt,
        const TagLib::Matroska::File& file);

} // namespace matroska

} // namespace taglib

} // namespace mixxx
#endif
