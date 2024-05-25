#pragma once

#include <apetag.h>

class QImage;

namespace mixxx {

class TrackMetadata;

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
