#pragma once

#include <taglib/infotag.h>

#include "track/taglib/trackmetadata_common.h"

namespace mixxx {

namespace taglib {

namespace riff {

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::RIFF::Info::Tag& tag);

} // namespace riff

} // namespace taglib

} // namespace mixxx
