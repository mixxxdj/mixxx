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

// Read metadata
// The general function readTag() is implicitly invoked
// from the specialized tag reading functions!
void readTrackMetadataFromTag(TrackMetadata* pTrackMetadata, const TagLib::Tag& tag);
void readTrackMetadataFromID3v2Tag(TrackMetadata* pTrackMetadata, const TagLib::ID3v2::Tag& tag);
void readTrackMetadataFromAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag);
void readTrackMetadataFromXiphComment(TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag);
void readTrackMetadataFromMP4Tag(TrackMetadata* pTrackMetadata, const TagLib::MP4::Tag& tag);

// Read cover art
// In order to avoid processing images when it's not
// needed (TIO building), we must process it separately.
bool readCoverArtFromID3v2Tag(QImage* pCoverArt, const TagLib::ID3v2::Tag& tag);
bool readCoverArtFromAPETag(QImage* pCoverArt, const TagLib::APE::Tag& tag);
bool readCoverArtFromXiphComment(QImage* pCoverArt, const TagLib::Ogg::XiphComment& tag);
bool readCoverArtFromMP4Tag(QImage* pCoverArt, const TagLib::MP4::Tag& tag);

// Write metadata
// The general function writeTag() is implicitly invoked
// from the specialized tag writing functions!
bool writeTrackMetadataIntoID3v2Tag(TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoAPETag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoXiphComment(TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoMP4Tag(TagLib::MP4::Tag* pTag, const TrackMetadata& trackMetadata);

} //namespace Mixxx

#endif
