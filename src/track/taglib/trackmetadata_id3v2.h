#pragma once

#include <taglib/id3v2tag.h>

#include "track/taglib/trackmetadata_common.h"

namespace mixxx {

namespace taglib {

namespace id3v2 {

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::ID3v2::Tag& tag,
        bool resetMissingTagMetadata);

bool importCoverImageFromTag(
        QImage* pCoverArt,
        const TagLib::ID3v2::Tag& tag);

bool exportTrackMetadataIntoTag(
        TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata);

} // namespace id3v2

} // namespace taglib

} // namespace mixxx
