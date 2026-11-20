#pragma once

#include <id3v2tag.h>

#include <optional>

class QImage;

namespace mixxx {

class TrackMetadata;

namespace taglib {

namespace id3v2 {

/// Import rating from ID3v2 tag (FMPS_Rating TXXX frame preferred, POPM fallback)
/// Returns std::nullopt if no rating is found, or a value 0-5 if found
std::optional<int> importRatingFromTag(const TagLib::ID3v2::Tag& tag);

/// Export rating to ID3v2 tag as FMPS_Rating TXXX frame
/// Rating should be 0-5, where 0 means unrated (removes existing frame)
/// Returns true on success, false on invalid rating
bool exportRatingIntoTag(
        TagLib::ID3v2::Tag* pTag,
        int rating);

/// Import the play count from ID3v2 tag (FMPS_Playcount TXXX frame
/// preferred, counter of a POPM frame as fallback)
std::optional<int> importPlayCountFromTag(const TagLib::ID3v2::Tag& tag);

/// Export the play count to ID3v2 tag as FMPS_Playcount TXXX frame
/// Returns true on success, false on a negative play count
bool exportPlayCountIntoTag(
        TagLib::ID3v2::Tag* pTag,
        int playCount);

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
