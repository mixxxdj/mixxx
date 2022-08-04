#pragma once

#include <taglib/apetag.h>

#include "track/taglib/trackmetadata_common.h"

namespace mixxx {

namespace taglib {

namespace ape {

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::APE::Tag& tag,
        bool resetMissingTagMetadata);

bool importCoverImageFromTag(
        QImage* pCoverArt,
        const TagLib::APE::Tag& tag);

bool exportTrackMetadataIntoTag(
        TagLib::APE::Tag* pTag,
        const TrackMetadata& trackMetadata);

} // namespace ape

} // namespace taglib

} // namespace mixxx
