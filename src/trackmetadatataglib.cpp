#include "trackmetadatataglib.h"

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

#include <QDebug>

namespace Mixxx
{

// static
namespace
{
    const bool kDebugMetadata = false;

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


bool readAudioProperties(TrackMetadata* pTrackMetadata, const TagLib::File& f) {
    if (kDebugMetadata) {
        qDebug() << "Parsing" << f.name();
    }

    if (f.isValid()) {
        const TagLib::AudioProperties *properties = f.audioProperties();
        if (properties) {
            pTrackMetadata->setChannels(properties->channels());
            pTrackMetadata->setSampleRate(properties->sampleRate());
            pTrackMetadata->setDuration(properties->length());
            pTrackMetadata->setBitrate(properties->bitrate());

            if (kDebugMetadata) {
                qDebug() << "TagLib"
                        << "channels" << pTrackMetadata->getChannels()
                        << "sampleRate" << pTrackMetadata->getSampleRate()
                        << "bitrate" << pTrackMetadata->getBitrate()
                        << "duration" << pTrackMetadata->getDuration();
            }

            return true;
        }
    }
    return false;
}

void readTag(TrackMetadata* pTrackMetadata, const TagLib::Tag& tag) {
    pTrackMetadata->setTitle(toQString(tag.title()));
    pTrackMetadata->setArtist(toQString(tag.artist()));
    pTrackMetadata->setAlbum(toQString(tag.album()));
    pTrackMetadata->setComment(toQString(tag.comment()));
    pTrackMetadata->setGenre(toQString(tag.genre()));

    int iYear = tag.year();
    if (iYear > 0) {
        pTrackMetadata->setYear(QString("%1").arg(iYear));
    }

    int iTrack = tag.track();
    if (iTrack > 0) {
        pTrackMetadata->setTrackNumber(QString("%1").arg(iTrack));
    }

    if (kDebugMetadata) {
        qDebug() << "TagLib"
                << "title" << pTrackMetadata->getTitle()
                << "artist" << pTrackMetadata->getArtist()
                << "album" << pTrackMetadata->getAlbum()
                << "comment" << pTrackMetadata->getComment()
                << "genre" << pTrackMetadata->getGenre()
                << "year" << pTrackMetadata->getYear()
                << "trackNumber" << pTrackMetadata->getTrackNumber();
    }
}

void readID3v2Tag(TrackMetadata* pTrackMetadata, const TagLib::ID3v2::Tag& id3v2) {

    // Print every frame in the file.
    if (kDebugMetadata) {
        TagLib::ID3v2::FrameList::ConstIterator it = id3v2.frameList().begin();
        for(; it != id3v2.frameList().end(); it++) {
            qDebug() << "ID3V2" << (*it)->frameID().data() << "-"
                    << toQString((*it)->toString());
        }
    }

    readTag(pTrackMetadata, id3v2);

    TagLib::ID3v2::FrameList bpmFrame = id3v2.frameListMap()["TBPM"];
    if (!bpmFrame.isEmpty()) {
        QString sBpm = toQString(bpmFrame.front()->toString());
        pTrackMetadata->setBpmString(sBpm);
    }

    TagLib::ID3v2::FrameList keyFrame = id3v2.frameListMap()["TKEY"];
    if (!keyFrame.isEmpty()) {
        QString sKey = toQString(keyFrame.front()->toString());
        pTrackMetadata->setKey(sKey);
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
                pTrackMetadata->setReplayGainString(sReplayGain);
            }
            //Prefer track gain over album gain.
            if (desc == "replaygain_track_gain") {
                QString sReplayGain = toQString(ReplayGainframe->fieldList()[1]);
                pTrackMetadata->setReplayGainString(sReplayGain);
            }
        }
    }

    TagLib::ID3v2::FrameList albumArtistFrame = id3v2.frameListMap()["TPE2"];
    if (!albumArtistFrame.isEmpty()) {
        QString sAlbumArtist = toQString(albumArtistFrame.front()->toString());
        pTrackMetadata->setAlbumArtist(sAlbumArtist);

        if (pTrackMetadata->getArtist().length() == 0) {
            pTrackMetadata->setArtist(sAlbumArtist);
        }
    }
    TagLib::ID3v2::FrameList originalAlbumFrame = id3v2.frameListMap()["TOAL"];
    if (pTrackMetadata->getAlbum().length() == 0 && !originalAlbumFrame.isEmpty()) {
        QString sOriginalAlbum = TStringToQString(originalAlbumFrame.front()->toString());
        pTrackMetadata->setAlbum(sOriginalAlbum);
    }

    TagLib::ID3v2::FrameList composerFrame = id3v2.frameListMap()["TCOM"];
    if (!composerFrame.isEmpty()) {
        QString sComposer = toQString(composerFrame.front()->toString());
        pTrackMetadata->setComposer(sComposer);
    }

    TagLib::ID3v2::FrameList groupingFrame = id3v2.frameListMap()["TIT1"];
    if (!groupingFrame.isEmpty()) {
        QString sGrouping = toQString(groupingFrame.front()->toString());
        pTrackMetadata->setGrouping(sGrouping);
    }
}

void readAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& ape) {
    if (kDebugMetadata) {
        for(TagLib::APE::ItemListMap::ConstIterator it = ape.itemListMap().begin();
                it != ape.itemListMap().end(); ++it) {
                qDebug() << "APE" << toQString((*it).first) << "-" << toQString((*it).second.toString());
        }
    }

    readTag(pTrackMetadata, ape);

    if (ape.itemListMap().contains("BPM")) {
        pTrackMetadata->setBpmString(toQString(ape.itemListMap()["BPM"]));
    }

    if (ape.itemListMap().contains("REPLAYGAIN_ALBUM_GAIN")) {
        pTrackMetadata->setReplayGainString(toQString(ape.itemListMap()["REPLAYGAIN_ALBUM_GAIN"]));
    }
    //Prefer track gain over album gain.
    if (ape.itemListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
        pTrackMetadata->setReplayGainString(toQString(ape.itemListMap()["REPLAYGAIN_TRACK_GAIN"]));
    }

    if (ape.itemListMap().contains("Album Artist")) {
        pTrackMetadata->setAlbumArtist(toQString(ape.itemListMap()["Album Artist"]));
    }

    if (ape.itemListMap().contains("Composer")) {
        pTrackMetadata->setComposer(toQString(ape.itemListMap()["Composer"]));
    }

    if (ape.itemListMap().contains("Grouping")) {
        pTrackMetadata->setGrouping(toQString(ape.itemListMap()["Grouping"]));
    }
}

void readXiphComment(TrackMetadata* pTrackMetadata, const TagLib::Ogg::XiphComment& xiph) {
    if (kDebugMetadata) {
        for (TagLib::Ogg::FieldListMap::ConstIterator it = xiph.fieldListMap().begin();
                it != xiph.fieldListMap().end(); ++it) {
            qDebug() << "XIPH" << toQString((*it).first) << "-" << toQString((*it).second.toString());
        }
    }

    readTag(pTrackMetadata, xiph);

    // Some tags use "BPM" so check for that.
    if (xiph.fieldListMap().contains("BPM")) {
        pTrackMetadata->setBpmString(toQString(xiph.fieldListMap()["BPM"]));
    }

    // Give preference to the "TEMPO" tag which seems to be more standard
    if (xiph.fieldListMap().contains("TEMPO")) {
        pTrackMetadata->setBpmString(toQString(xiph.fieldListMap()["TEMPO"]));
    }

    if (xiph.fieldListMap().contains("REPLAYGAIN_ALBUM_GAIN")) {
        pTrackMetadata->setReplayGainString(toQString(xiph.fieldListMap()["REPLAYGAIN_ALBUM_GAIN"]));
    }
    //Prefer track gain over album gain.
    if (xiph.fieldListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
        pTrackMetadata->setReplayGainString(toQString(xiph.fieldListMap()["REPLAYGAIN_TRACK_GAIN"]));
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
        pTrackMetadata->setKey(toQString(xiph.fieldListMap()["KEY"]));
    }
    if (pTrackMetadata->getKey().isEmpty() && xiph.fieldListMap().contains("INITIALKEY")) {
        pTrackMetadata->setKey(toQString(xiph.fieldListMap()["INITIALKEY"]));
    }

    if (xiph.fieldListMap().contains("ALBUMARTIST")) {
        pTrackMetadata->setAlbumArtist(toQString(xiph.fieldListMap()["ALBUMARTIST"]));
    } else {
        // try alternative field name
        if (xiph.fieldListMap().contains("ALBUM_ARTIST")) {
            pTrackMetadata->setAlbumArtist(toQString(xiph.fieldListMap()["ALBUM_ARTIST"]));
        }
    }

    if (xiph.fieldListMap().contains("COMPOSER")) {
        pTrackMetadata->setComposer(toQString(xiph.fieldListMap()["COMPOSER"]));
    }

    if (xiph.fieldListMap().contains("GROUPING")) {
        pTrackMetadata->setGrouping(toQString(xiph.fieldListMap()["GROUPING"]));
    }
}

void readMP4Tag(TrackMetadata* pTrackMetadata, /*const*/ TagLib::MP4::Tag& mp4) {
    if (kDebugMetadata) {
        for(TagLib::MP4::ItemListMap::ConstIterator it = mp4.itemListMap().begin();
            it != mp4.itemListMap().end(); ++it) {
            qDebug() << "MP4" << toQString((*it).first) << "-"
                     << toQString((*it).second);
        }
    }

    readTag(pTrackMetadata, mp4);

    // Get BPM
    if (mp4.itemListMap().contains("tmpo")) {
        pTrackMetadata->setBpmString(toQString(mp4.itemListMap()["tmpo"]));
    } else if (mp4.itemListMap().contains("----:com.apple.iTunes:BPM")) {
        // This is an alternate way of storing BPM.
        pTrackMetadata->setBpmString(toQString(mp4.itemListMap()["----:com.apple.iTunes:BPM"]));
    }

    // Get Album Artist
    if (mp4.itemListMap().contains("aART")) {
        pTrackMetadata->setAlbumArtist(toQString(mp4.itemListMap()["aART"]));
    }

    // Get Composer
    if (mp4.itemListMap().contains("\251wrt")) {
        pTrackMetadata->setComposer(toQString(mp4.itemListMap()["\251wrt"]));
    }

    // Get Grouping
    if (mp4.itemListMap().contains("\251grp")) {
        pTrackMetadata->setGrouping(toQString(mp4.itemListMap()["\251grp"]));
    }

    // Get KEY (conforms to Rapid Evolution)
    if (mp4.itemListMap().contains("----:com.apple.iTunes:KEY")) {
        QString key = toQString(
            mp4.itemListMap()["----:com.apple.iTunes:KEY"]);
        pTrackMetadata->setKey(key);
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

QImage readID3v2TagCover(const TagLib::ID3v2::Tag& id3v2) {
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

QImage readAPETagCover(const TagLib::APE::Tag& ape) {
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

QImage readXiphCommentCover(const TagLib::Ogg::XiphComment& xiph) {
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

QImage readMP4TagCover(/*const*/ TagLib::MP4::Tag& mp4) {
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
