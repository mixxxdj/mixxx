#ifndef TRACKMETADATATAGLIB_H
#define TRACKMETADATATAGLIB_H

#include <taglib/apetag.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>

#include <QImage>

#include "track/trackmetadata.h"
#include "util/result.h"

namespace Mixxx {

// Read both track metadata and cover art of supported file types.
// Both parameters are optional and might be NULL.
Result readTrackMetadataAndCoverArtFromFile(TrackMetadata* pTrackMetadata, QImage* pCoverArt, QString fileName);

// Write track metadata into the file with the given name
Result writeTrackMetadataIntoFile(const TrackMetadata& trackMetadata, QString fileName);

// Low-level tag read/write functions are exposed only for testing purposes!
void readTrackMetadataFromID3v2Tag(TrackMetadata* pTrackMetadata, const TagLib::ID3v2::Tag& tag);
void readTrackMetadataFromAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag);
void readTrackMetadataFromXiphComment(TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag);
void readTrackMetadataFromMP4Tag(TrackMetadata* pTrackMetadata, const TagLib::MP4::Tag& tag);
bool writeTrackMetadataIntoID3v2Tag(TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoAPETag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoXiphComment(TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoMP4Tag(TagLib::MP4::Tag* pTag, const TrackMetadata& trackMetadata);

} //namespace Mixxx

#endif
