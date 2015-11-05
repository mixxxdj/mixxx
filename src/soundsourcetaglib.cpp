/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "soundsourcetaglib.h"

#include <taglib/tag.h>
#include <taglib/audioproperties.h>
#include <taglib/vorbisfile.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v1tag.h>
#include <taglib/tmap.h>
#include <taglib/tstringlist.h>
#include <taglib/textidentificationframe.h>
#include <taglib/wavpackfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacpicture.h>


#include <QtDebug>


namespace Mixxx
{

// static
namespace
{
    const bool s_bDebugMetadata = false;

    // Taglib strings can be NULL and using it could cause some segfaults,
    // so in this case it will return a QString()
    inline
    QString toQString(const TagLib::String& tString) {
        if (tString.isNull()) {
            return QString();
        } else {
            return TStringToQString(tString);
        }
    }

    // Concatenates the elements of a TagLib string list
    // into a single string.
    inline
    QString toQString(const TagLib::StringList& strList) {
        return toQString(strList.toString());
    }

    // Concatenates the string list of an MP4 atom
    // into a single string
    inline
    QString toQString(const TagLib::MP4::Item& mp4Item) {
        return toQString(mp4Item.toStringList());
    }

    inline
    QString toQString(const TagLib::APE::Item& apeItem) {
        return toQString(apeItem.toString());
    }
}


bool readFileHeader(SoundSource* pSoundSource, const TagLib::File& f) {
    if (s_bDebugMetadata) {
        qDebug() << "Parsing" << pSoundSource->getFilename();
    }

    if (f.isValid()) {
        const TagLib::AudioProperties *properties = f.audioProperties();
        if (properties) {
            pSoundSource->setDuration(properties->length());
            pSoundSource->setBitrate(properties->bitrate());
            pSoundSource->setSampleRate(properties->sampleRate());
            pSoundSource->setChannels(properties->channels());

            if (s_bDebugMetadata) {
                qDebug() << "TagLib"
                        << "duration" << pSoundSource->getDuration()
                        << "bitrate" << pSoundSource->getBitrate()
                        << "sampleRate" << pSoundSource->getSampleRate()
                        << "channels" << pSoundSource->getChannels();
            }

            return true;
        }
    }
    return false;
}

void readTag(SoundSource* pSoundSource, const TagLib::Tag& tag) {
    pSoundSource->setTitle(toQString(tag.title()));
    pSoundSource->setArtist(toQString(tag.artist()));
    pSoundSource->setAlbum(toQString(tag.album()));
    pSoundSource->setComment(toQString(tag.comment()));
    pSoundSource->setGenre(toQString(tag.genre()));

    int iYear = tag.year();
    if (iYear > 0) {
        pSoundSource->setYear(QString("%1").arg(iYear));
    }

    int iTrack = tag.track();
    if (iTrack > 0) {
        pSoundSource->setTrackNumber(QString("%1").arg(iTrack));
    }

    if (s_bDebugMetadata) {
        qDebug() << "TagLib"
                << "title" << pSoundSource->getTitle()
                << "artist" << pSoundSource->getArtist()
                << "album" << pSoundSource->getAlbum()
                << "comment" << pSoundSource->getComment()
                << "genre" << pSoundSource->getGenre()
                << "year" << pSoundSource->getYear()
                << "trackNumber" << pSoundSource->getTrackNumber();
    }
}

void readID3v2Tag(SoundSource* pSoundSource, const TagLib::ID3v2::Tag& id3v2) {

    // Print every frame in the file.
    if (s_bDebugMetadata) {
        TagLib::ID3v2::FrameList::ConstIterator it = id3v2.frameList().begin();
        for(; it != id3v2.frameList().end(); it++) {
            qDebug() << "ID3V2" << (*it)->frameID().data() << "-"
                    << toQString((*it)->toString());
        }
    }

    readTag(pSoundSource, id3v2);

    TagLib::ID3v2::FrameList bpmFrame = id3v2.frameListMap()["TBPM"];
    if (!bpmFrame.isEmpty()) {
        QString sBpm = toQString(bpmFrame.front()->toString());
        pSoundSource->setBpmString(sBpm);
    }

    TagLib::ID3v2::FrameList keyFrame = id3v2.frameListMap()["TKEY"];
    if (!keyFrame.isEmpty()) {
        QString sKey = toQString(keyFrame.front()->toString());
        pSoundSource->setKey(sKey);
    }

    // Foobar2000-style ID3v2.3.0 tags
    // TODO: Check if everything is ok.
    TagLib::ID3v2::FrameList frames = id3v2.frameListMap()["TXXX"];
    for (TagLib::ID3v2::FrameList::Iterator it = frames.begin(); it != frames.end(); ++it) {
        TagLib::ID3v2::UserTextIdentificationFrame* ReplayGainframe =
                dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
        if (ReplayGainframe && ReplayGainframe->fieldList().size() >= 2) {
            QString desc = toQString(ReplayGainframe->description()).toLower();
            if (desc == "replaygain_album_gain") {
                QString sReplayGain = toQString(ReplayGainframe->fieldList()[1]);
                pSoundSource->setReplayGainString(sReplayGain);
            }
            //Prefer track gain over album gain.
            if (desc == "replaygain_track_gain") {
                QString sReplayGain = toQString(ReplayGainframe->fieldList()[1]);
                pSoundSource->setReplayGainString(sReplayGain);
            }
        }
    }

    TagLib::ID3v2::FrameList albumArtistFrame = id3v2.frameListMap()["TPE2"];
    if (!albumArtistFrame.isEmpty()) {
        QString sAlbumArtist = toQString(albumArtistFrame.front()->toString());
        pSoundSource->setAlbumArtist(sAlbumArtist);

        if (pSoundSource->getArtist().length() == 0) {
            pSoundSource->setArtist(sAlbumArtist);
        }
    }
    TagLib::ID3v2::FrameList originalAlbumFrame = id3v2.frameListMap()["TOAL"];
    if (pSoundSource->getAlbum().length() == 0 && !originalAlbumFrame.isEmpty()) {
        QString sOriginalAlbum = TStringToQString(originalAlbumFrame.front()->toString());
        pSoundSource->setAlbum(sOriginalAlbum);
    }

    TagLib::ID3v2::FrameList composerFrame = id3v2.frameListMap()["TCOM"];
    if (!composerFrame.isEmpty()) {
        QString sComposer = toQString(composerFrame.front()->toString());
        pSoundSource->setComposer(sComposer);
    }

    TagLib::ID3v2::FrameList groupingFrame = id3v2.frameListMap()["TIT1"];
    if (!groupingFrame.isEmpty()) {
        QString sGrouping = toQString(groupingFrame.front()->toString());
        pSoundSource->setGrouping(sGrouping);
    }
}

void readAPETag(SoundSource* pSoundSource, const TagLib::APE::Tag& ape) {
    if (s_bDebugMetadata) {
        for(TagLib::APE::ItemListMap::ConstIterator it = ape.itemListMap().begin();
                it != ape.itemListMap().end(); ++it) {
                qDebug() << "APE" << toQString((*it).first) << "-" << toQString((*it).second.toString());
        }
    }

    readTag(pSoundSource, ape);

    if (ape.itemListMap().contains("BPM")) {
        pSoundSource->setBpmString(toQString(ape.itemListMap()["BPM"]));
    }

    if (ape.itemListMap().contains("REPLAYGAIN_ALBUM_GAIN")) {
        pSoundSource->setReplayGainString(toQString(ape.itemListMap()["REPLAYGAIN_ALBUM_GAIN"]));
    }
    //Prefer track gain over album gain.
    if (ape.itemListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
        pSoundSource->setReplayGainString(toQString(ape.itemListMap()["REPLAYGAIN_TRACK_GAIN"]));
    }

    if (ape.itemListMap().contains("Album Artist")) {
        pSoundSource->setAlbumArtist(toQString(ape.itemListMap()["Album Artist"]));
    }

    if (ape.itemListMap().contains("Composer")) {
        pSoundSource->setComposer(toQString(ape.itemListMap()["Composer"]));
    }

    if (ape.itemListMap().contains("Grouping")) {
        pSoundSource->setGrouping(toQString(ape.itemListMap()["Grouping"]));
    }
}

void readXiphComment(SoundSource* pSoundSource, const TagLib::Ogg::XiphComment& xiph) {
    if (s_bDebugMetadata) {
        for (TagLib::Ogg::FieldListMap::ConstIterator it = xiph.fieldListMap().begin();
                it != xiph.fieldListMap().end(); ++it) {
            qDebug() << "XIPH" << toQString((*it).first) << "-" << toQString((*it).second.toString());
        }
    }

    readTag(pSoundSource, xiph);

    // Some tags use "BPM" so check for that.
    if (xiph.fieldListMap().contains("BPM")) {
        pSoundSource->setBpmString(toQString(xiph.fieldListMap()["BPM"]));
    }

    // Give preference to the "TEMPO" tag which seems to be more standard
    if (xiph.fieldListMap().contains("TEMPO")) {
        pSoundSource->setBpmString(toQString(xiph.fieldListMap()["TEMPO"]));
    }

    if (xiph.fieldListMap().contains("REPLAYGAIN_ALBUM_GAIN")) {
        pSoundSource->setReplayGainString(toQString(xiph.fieldListMap()["REPLAYGAIN_ALBUM_GAIN"]));
    }
    //Prefer track gain over album gain.
    if (xiph.fieldListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
        pSoundSource->setReplayGainString(toQString(xiph.fieldListMap()["REPLAYGAIN_TRACK_GAIN"]));
    }

    /*
     * Reading key code information
     * Unlike, ID3 tags, there's no standard or recommendation on how to store 'key' code
     *
     * Luckily, there are only a few tools for that, e.g., Rapid Evolution (RE).
     * Assuming no distinction between start and end key, RE uses a "INITIALKEY"
     * or a "KEY" vorbis comment.
     */
    if (xiph.fieldListMap().contains("KEY")) {
        pSoundSource->setKey(toQString(xiph.fieldListMap()["KEY"]));
    }
    if (pSoundSource->getKey().isEmpty() && xiph.fieldListMap().contains("INITIALKEY")) {
        pSoundSource->setKey(toQString(xiph.fieldListMap()["INITIALKEY"]));
    }

    if (xiph.fieldListMap().contains("ALBUMARTIST")) {
        pSoundSource->setAlbumArtist(toQString(xiph.fieldListMap()["ALBUMARTIST"]));
    } else {
        // try alternative field name
        if (xiph.fieldListMap().contains("ALBUM_ARTIST")) {
            pSoundSource->setAlbumArtist(toQString(xiph.fieldListMap()["ALBUM_ARTIST"]));
        }
    }

    if (xiph.fieldListMap().contains("COMPOSER")) {
        pSoundSource->setComposer(toQString(xiph.fieldListMap()["COMPOSER"]));
    }

    if (xiph.fieldListMap().contains("GROUPING")) {
        pSoundSource->setGrouping(toQString(xiph.fieldListMap()["GROUPING"]));
    }
}

void readMP4Tag(SoundSource* pSoundSource, /*const*/ TagLib::MP4::Tag& mp4) {
    if (s_bDebugMetadata) {
        for(TagLib::MP4::ItemListMap::ConstIterator it = mp4.itemListMap().begin();
            it != mp4.itemListMap().end(); ++it) {
            qDebug() << "MP4" << toQString((*it).first) << "-"
                     << toQString((*it).second);
        }
    }

    readTag(pSoundSource, mp4);

    // Get BPM
    if (mp4.itemListMap().contains("tmpo")) {
        pSoundSource->setBpmString(toQString(mp4.itemListMap()["tmpo"]));
    } else if (mp4.itemListMap().contains("----:com.apple.iTunes:BPM")) {
        // This is an alternate way of storing BPM.
        pSoundSource->setBpmString(toQString(mp4.itemListMap()["----:com.apple.iTunes:BPM"]));
    }

    // Get Album Artist
    if (mp4.itemListMap().contains("aART")) {
        pSoundSource->setAlbumArtist(toQString(mp4.itemListMap()["aART"]));
    }

    // Get Composer
    if (mp4.itemListMap().contains("\251wrt")) {
        pSoundSource->setComposer(toQString(mp4.itemListMap()["\251wrt"]));
    }

    // Get Grouping
    if (mp4.itemListMap().contains("\251grp")) {
        pSoundSource->setGrouping(toQString(mp4.itemListMap()["\251grp"]));
    }

    // Get KEY (conforms to Rapid Evolution)
    if (mp4.itemListMap().contains("----:com.apple.iTunes:KEY")) {
        QString key = toQString(
            mp4.itemListMap()["----:com.apple.iTunes:KEY"]);
        pSoundSource->setKey(key);
    }

    // Apparently iTunes stores replaygain in this property.
    if (mp4.itemListMap().contains("----:com.apple.iTunes:replaygain_album_gain")) {
        // TODO(XXX) find tracks with this property and check what it looks
        // like.
        //QString replaygain = toQString(mp4.itemListMap()["----:com.apple.iTunes:replaygain_album_gain"]);
    }
    //Prefer track gain over album gain.
    if (mp4.itemListMap().contains("----:com.apple.iTunes:replaygain_track_gain")) {
        // TODO(XXX) find tracks with this property and check what it looks
        // like.
        //QString replaygain = toQString(mp4.itemListMap()["----:com.apple.iTunes:replaygain_track_gain"]);
    }
}

QImage getCoverInID3v2Tag(const TagLib::ID3v2::Tag& id3v2) {
    QImage coverArt;
    TagLib::ID3v2::FrameList covertArtFrame = id3v2.frameListMap()["APIC"];
    if (!covertArtFrame.isEmpty()) {
        TagLib::ID3v2::AttachedPictureFrame* picframe = static_cast
                <TagLib::ID3v2::AttachedPictureFrame*>(covertArtFrame.front());
        TagLib::ByteVector data = picframe->picture();
        coverArt = QImage::fromData(
            reinterpret_cast<const uchar *>(data.data()), data.size());
    }
    return coverArt;
}

QImage getCoverInAPETag(const TagLib::APE::Tag& ape) {
    QImage coverArt;
    if (ape.itemListMap().contains("COVER ART (FRONT)"))
    {
        const TagLib::ByteVector nullStringTerminator(1, 0);
        TagLib::ByteVector item = ape.itemListMap()["COVER ART (FRONT)"].value();
        int pos = item.find(nullStringTerminator);  // skip the filename
        if (++pos > 0) {
            const TagLib::ByteVector& data = item.mid(pos);
            coverArt = QImage::fromData(
                reinterpret_cast<const uchar *>(data.data()), data.size());
        }
    }
    return coverArt;
}

QImage getCoverInXiphComment(const TagLib::Ogg::XiphComment& xiph) {
    QImage coverArt;
    if (xiph.fieldListMap().contains("METADATA_BLOCK_PICTURE")) {
        QByteArray data(QByteArray::fromBase64(
            xiph.fieldListMap()["METADATA_BLOCK_PICTURE"].front().toCString()));
        TagLib::ByteVector tdata(data.data(), data.size());
        TagLib::FLAC::Picture p(tdata);
        data = QByteArray(p.data().data(), p.data().size());
        coverArt = QImage::fromData(data);
    } else if (xiph.fieldListMap().contains("COVERART")) {
        QByteArray data(QByteArray::fromBase64(
            xiph.fieldListMap()["COVERART"].toString().toCString()));
        coverArt = QImage::fromData(data);
    }
    return coverArt;
}

QImage getCoverInMP4Tag(/*const*/ TagLib::MP4::Tag& mp4) {
    QImage coverArt;
    if (mp4.itemListMap().contains("covr")) {
        TagLib::MP4::CoverArtList coverArtList = mp4.itemListMap()["covr"]
                                                        .toCoverArtList();
        TagLib::ByteVector data = coverArtList.front().data();
        coverArt = QImage::fromData(
            reinterpret_cast<const uchar *>(data.data()), data.size());
    }
    return coverArt;
}

} //namespace Mixxx
