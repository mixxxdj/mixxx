#pragma once

#include <taglib/xiphcomment.h>

#include <QImage>

#include "track/taglib/trackmetadata_common.h"
#include "track/taglib/trackmetadata_file.h"

namespace TagLib {
namespace FLAC {
class Picture;
} // namespace FLAC
template<class T>
class List;
} // namespace TagLib

namespace mixxx {
class TrackMetadata;

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
        FileType fileType);

bool exportTrackMetadataIntoTag(
        TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata,
        FileType fileType);

} // namespace xiph

} // namespace taglib

} // namespace mixxx
