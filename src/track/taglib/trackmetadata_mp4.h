#pragma once

#include <mp4tag.h>

#include <optional>

class QImage;

namespace mixxx {

class TrackMetadata;

namespace taglib {

namespace mp4 {

/// Import rating from MP4 tag (FMPS_Rating freeform atom)
/// Returns std::nullopt if no rating is found, or a value 0-5 if found
std::optional<int> importRatingFromTag(const TagLib::MP4::Tag& tag);

/// Export rating to MP4 tag as FMPS_Rating freeform atom
/// Rating should be 0-5, where 0 means unrated (removes existing atom)
/// Returns true on success, false on invalid rating
bool exportRatingIntoTag(
        TagLib::MP4::Tag* pTag,
        int rating);

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
