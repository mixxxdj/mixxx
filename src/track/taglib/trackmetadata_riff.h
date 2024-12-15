#pragma once

#include <infotag.h>

class TrackMetadata;

namespace mixxx {

namespace taglib {

namespace riff {

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::RIFF::Info::Tag& tag);

} // namespace riff

} // namespace taglib

} // namespace mixxx
