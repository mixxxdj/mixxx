#pragma once

#include <xiphcomment.h>

#include <optional>

#include "track/taglib/trackmetadata_file.h"

namespace mixxx {

namespace taglib {

namespace xiph {

/// Import rating from Xiph/Vorbis comment (FMPS_RATING field)
/// Returns std::nullopt if no rating is found, or a value 0-5 if found
std::optional<int> importRatingFromTag(const TagLib::Ogg::XiphComment& tag);

/// Export rating to Xiph/Vorbis comment as FMPS_RATING field
/// Rating should be 0-5, where 0 means unrated (removes existing field)
/// Returns true on success, false on invalid rating
bool exportRatingIntoTag(
        TagLib::Ogg::XiphComment* pTag,
        int rating);

bool importCoverImageFromTag(
        QImage* pCoverArt,
        TagLib::Ogg::XiphComment& tag);

QImage importCoverImageFromPictureList(
        const TagLib::List<TagLib::FLAC::Picture*>& pictures);

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag,
        FileType fileType,
        bool resetMissingTagMetadata);

bool exportTrackMetadataIntoTag(
        TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata,
        FileType fileType);

} // namespace xiph

} // namespace taglib

} // namespace mixxx
