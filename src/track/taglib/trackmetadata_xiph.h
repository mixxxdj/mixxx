#pragma once

#include <taglib/xiphcomment.h>

#include "track/taglib/trackmetadata_common.h"

namespace mixxx {

namespace taglib {

namespace xiph {

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
