#pragma once

#include <id3v2tag.h>

class QImage;

namespace mixxx {

class TrackMetadata;

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
