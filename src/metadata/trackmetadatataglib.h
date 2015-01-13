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

#include <taglib/apetag.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>

#include <QImage>

namespace Mixxx {

// Read common audio properties of a file
bool readAudioProperties(TrackMetadata* pTrackMetadata,
        const TagLib::File& file);

// Read metadata
// The general function readTag() is implicitly invoked
// from the specialized tag reading functions!
void readTag(TrackMetadata* pTrackMetadata, const TagLib::Tag& tag);
void readID3v2Tag(TrackMetadata* pTrackMetadata, const TagLib::ID3v2::Tag& tag);
void readAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag);
void readXiphComment(TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag);
void readMP4Tag(TrackMetadata* pTrackMetadata, /*const*/TagLib::MP4::Tag& tag);

// Write metadata
// The general function writeTag() is implicitly invoked
// from the specialized tag writing functions!
bool writeID3v2Tag(TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata);
bool writeAPETag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata);
bool writeXiphComment(TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata);
bool writeMP4Tag(TagLib::MP4::Tag* pTag, const TrackMetadata& trackMetadata);

// Read cover art
// In order to avoid processing images when it's not
// needed (TIO building), we must process it separately.
QImage readID3v2TagCover(const TagLib::ID3v2::Tag& tag);
QImage readAPETagCover(const TagLib::APE::Tag& tag);
QImage readXiphCommentCover(const TagLib::Ogg::XiphComment& tag);
QImage readMP4TagCover(/*const*/TagLib::MP4::Tag& tag);

} //namespace Mixxx

#endif
