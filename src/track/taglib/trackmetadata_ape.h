#pragma once

#include <apetag.h>

#include <optional>

class QImage;

namespace mixxx {

class TrackMetadata;

namespace taglib {

namespace ape {

/// Import rating from APE tag (FMPS_Rating item)
/// Returns std::nullopt if no rating is found, or a value 0-5 if found
std::optional<int> importRatingFromTag(const TagLib::APE::Tag& tag);

/// Export rating to APE tag as FMPS_Rating item
/// Rating should be 0-5, where 0 means unrated (removes existing item)
/// Returns true on success, false on invalid rating
bool exportRatingIntoTag(
        TagLib::APE::Tag* pTag,
        int rating);

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
