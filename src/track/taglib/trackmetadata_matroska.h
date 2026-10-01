#pragma once

// TagLib < 2.2 has no Matroska module; the header only exists from 2.2 on.
// taglib.h defines the TAGLIB_* version macros; include it first so the
// guard below works even when this header is the first TagLib include.
#include <taglib.h>
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
