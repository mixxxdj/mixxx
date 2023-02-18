#pragma once

#include <taglib/mp4tag.h>

#include "track/taglib/trackmetadata_common.h"

namespace mixxx {

namespace taglib {

namespace mp4 {

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::MP4::Tag& tag,
        bool resetMissingTagMetadata);

bool importCoverImageFromTag(
        QImage* pCoverArt,
        const TagLib::MP4::Tag& tag);

bool exportTrackMetadataIntoTag(
        TagLib::MP4::Tag* pTag,
        const TrackMetadata& trackMetadata);

} // namespace mp4

} // namespace taglib

} // namespace mixxx
