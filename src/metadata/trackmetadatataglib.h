/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCETAGLIB_H
#define SOUNDSOURCETAGLIB_H

#include "metadata/trackmetadata.h"
#include "util/defs.h" // Result

#include <taglib/apetag.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>

#include <QImage>

namespace Mixxx {

// Read track metadata of supported file types
Result readTrackMetadataFromFile(TrackMetadata* pTrackMetadata, QString fileName);

// Read cover art of supported file types
Result readCoverArtFromFile(QImage* pCoverArt, QString fileName);

// Read both track metadata and cover art of supported file types
//(both parameters are optional and might be NULL)
Result readTrackMetadataAndCoverArtFromFile(TrackMetadata* pTrackMetadata, QImage* pCoverArt, QString fileName);

// Low-level tag read/write functions for testing purposes
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
