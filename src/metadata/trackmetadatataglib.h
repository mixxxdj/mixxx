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

#include <taglib/tfile.h>
#include <taglib/apetag.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>

#include <QImage>

namespace Mixxx
{
    bool readAudioProperties(TrackMetadata* pTrackMetadata, const TagLib::File& taglibFile);

    // Read generic tags (will implicitly be invoked from the specialized functions)
    void readTag(TrackMetadata* pTrackMetadata, const TagLib::Tag& tag);

    // Read specific tags
    void readID3v2Tag(TrackMetadata* pTrackMetadata, const TagLib::ID3v2::Tag& id3v2Tag);
    void readAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& ape);
    void readXiphComment(TrackMetadata* pTrackMetadata, const TagLib::Ogg::XiphComment& xiph);
    void readMP4Tag(TrackMetadata* pTrackMetadata, /*const*/ TagLib::MP4::Tag& mp4);

    // In order to avoid processing images when it's not
    // needed (TIO building), we must process it separately.
    QImage readID3v2TagCover(const TagLib::ID3v2::Tag& id3v2);
    QImage readAPETagCover(const TagLib::APE::Tag& ape);
    QImage readXiphCommentCover(const TagLib::Ogg::XiphComment& xiph);
    QImage readMP4TagCover(/*const*/ TagLib::MP4::Tag& mp4);

} //namespace Mixxx


#endif
