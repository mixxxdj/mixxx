#pragma once

#include <mp4tag.h>

class QImage;

namespace mixxx {

class TrackMetadata;

namespace taglib {

namespace mp4 {

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::MP4::Tag& tag,
        bool resetMissingTagMetadata);

#ifdef __STEM__
void importStemInfo(TrackMetadata* pTrackMetadata,
        const TagLib::MP4::Tag& tag);
#endif

bool importCoverImageFromTag(
        QImage* pCoverArt,
        const TagLib::MP4::Tag& tag);

bool exportTrackMetadataIntoTag(
        TagLib::MP4::Tag* pTag,
        const TrackMetadata& trackMetadata);

#ifdef __STEM__
bool exportStemInfo(
        TagLib::MP4::Tag* pTag,
        const TrackMetadata& trackMetadata);
#endif

} // namespace mp4

} // namespace taglib

} // namespace mixxx
