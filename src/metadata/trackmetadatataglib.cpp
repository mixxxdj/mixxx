#include "metadata/trackmetadatataglib.h"

#include <taglib/tfile.h>
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

template<typename T>
inline
QString formatString(const T& value) {
    return QString("%1").arg(value);
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

void readID3v2Tag(TrackMetadata* pTrackMetadata, const TagLib::ID3v2::Tag& tag) {

    // Print every frame in the file.
    if (kDebugMetadata) {
        TagLib::ID3v2::FrameList::ConstIterator it = tag.frameList().begin();
        for(; it != tag.frameList().end(); it++) {
            qDebug() << "ID3V2" << (*it)->frameID().data() << "-"
                    << toQString((*it)->toString());
        }
    }

    readTag(pTrackMetadata, tag);

    TagLib::ID3v2::FrameList bpmFrame = tag.frameListMap()["TBPM"];
    if (!bpmFrame.isEmpty()) {
        QString sBpm = toQString(bpmFrame.front()->toString());
        pTrackMetadata->setBpmString(sBpm);
    }

    TagLib::ID3v2::FrameList keyFrame = tag.frameListMap()["TKEY"];
    if (!keyFrame.isEmpty()) {
        QString sKey = toQString(keyFrame.front()->toString());
        pTrackMetadata->setKey(sKey);
    }

    // Foobar2000-style ID3v2.3.0 tags
    // TODO: Check if everything is ok.
    TagLib::ID3v2::FrameList frames = tag.frameListMap()["TXXX"];
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

    TagLib::ID3v2::FrameList albumArtistFrame = tag.frameListMap()["TPE2"];
    if (!albumArtistFrame.isEmpty()) {
        QString sAlbumArtist = toQString(albumArtistFrame.front()->toString());
        pTrackMetadata->setAlbumArtist(sAlbumArtist);

        if (pTrackMetadata->getArtist().length() == 0) {
            pTrackMetadata->setArtist(sAlbumArtist);
        }
    }
    TagLib::ID3v2::FrameList originalAlbumFrame = tag.frameListMap()["TOAL"];
    if (pTrackMetadata->getAlbum().length() == 0 && !originalAlbumFrame.isEmpty()) {
        QString sOriginalAlbum = TStringToQString(originalAlbumFrame.front()->toString());
        pTrackMetadata->setAlbum(sOriginalAlbum);
    }

    TagLib::ID3v2::FrameList composerFrame = tag.frameListMap()["TCOM"];
    if (!composerFrame.isEmpty()) {
        QString sComposer = toQString(composerFrame.front()->toString());
        pTrackMetadata->setComposer(sComposer);
    }

    TagLib::ID3v2::FrameList groupingFrame = tag.frameListMap()["TIT1"];
    if (!groupingFrame.isEmpty()) {
        QString sGrouping = toQString(groupingFrame.front()->toString());
        pTrackMetadata->setGrouping(sGrouping);
    }
}

void readAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag) {
    if (kDebugMetadata) {
        for(TagLib::APE::ItemListMap::ConstIterator it = tag.itemListMap().begin();
                it != tag.itemListMap().end(); ++it) {
                qDebug() << "APE" << toQString((*it).first) << "-" << toQString((*it).second.toString());
        }
    }

    readTag(pTrackMetadata, tag);

    if (tag.itemListMap().contains("BPM")) {
        pTrackMetadata->setBpmString(toQString(tag.itemListMap()["BPM"]));
    }

    if (tag.itemListMap().contains("REPLAYGAIN_ALBUM_GAIN")) {
        pTrackMetadata->setReplayGainString(toQString(tag.itemListMap()["REPLAYGAIN_ALBUM_GAIN"]));
    }
    //Prefer track gain over album gain.
    if (tag.itemListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
        pTrackMetadata->setReplayGainString(toQString(tag.itemListMap()["REPLAYGAIN_TRACK_GAIN"]));
    }

    if (tag.itemListMap().contains("Album Artist")) {
        pTrackMetadata->setAlbumArtist(toQString(tag.itemListMap()["Album Artist"]));
    }

    if (tag.itemListMap().contains("Composer")) {
        pTrackMetadata->setComposer(toQString(tag.itemListMap()["Composer"]));
    }

    if (tag.itemListMap().contains("Grouping")) {
        pTrackMetadata->setGrouping(toQString(tag.itemListMap()["Grouping"]));
    }
}

void readXiphComment(TrackMetadata* pTrackMetadata, const TagLib::Ogg::XiphComment& tag) {
    if (kDebugMetadata) {
        for (TagLib::Ogg::FieldListMap::ConstIterator it = tag.fieldListMap().begin();
                it != tag.fieldListMap().end(); ++it) {
            qDebug() << "XIPH" << toQString((*it).first) << "-" << toQString((*it).second.toString());
        }
    }

    readTag(pTrackMetadata, tag);

    // Some tags use "BPM" so check for that.
    if (tag.fieldListMap().contains("BPM")) {
        pTrackMetadata->setBpmString(toQString(tag.fieldListMap()["BPM"]));
    }

    // Give preference to the "TEMPO" tag which seems to be more standard
    if (tag.fieldListMap().contains("TEMPO")) {
        pTrackMetadata->setBpmString(toQString(tag.fieldListMap()["TEMPO"]));
    }

    if (tag.fieldListMap().contains("REPLAYGAIN_ALBUM_GAIN")) {
        pTrackMetadata->setReplayGainString(toQString(tag.fieldListMap()["REPLAYGAIN_ALBUM_GAIN"]));
    }
    //Prefer track gain over album gain.
    if (tag.fieldListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
        pTrackMetadata->setReplayGainString(toQString(tag.fieldListMap()["REPLAYGAIN_TRACK_GAIN"]));
    }

    /*
     * Reading key code information
     * Unlike, ID3 tags, there's no standard or recommendation on how to store 'key' code
     *
     * Luckily, there are only a few tools for that, e.g., Rapid Evolution (RE).
     * Assuming no distinction between start and end key, RE uses a "INITIALKEY"
     * or a "KEY" vorbis comment.
     */
    if (tag.fieldListMap().contains("KEY")) {
        pTrackMetadata->setKey(toQString(tag.fieldListMap()["KEY"]));
    }
    if (pTrackMetadata->getKey().isEmpty() && tag.fieldListMap().contains("INITIALKEY")) {
        pTrackMetadata->setKey(toQString(tag.fieldListMap()["INITIALKEY"]));
    }

    if (tag.fieldListMap().contains("ALBUMARTIST")) {
        pTrackMetadata->setAlbumArtist(toQString(tag.fieldListMap()["ALBUMARTIST"]));
    } else {
        // try alternative field name
        if (tag.fieldListMap().contains("ALBUM_ARTIST")) {
            pTrackMetadata->setAlbumArtist(toQString(tag.fieldListMap()["ALBUM_ARTIST"]));
        }
    }

    if (tag.fieldListMap().contains("COMPOSER")) {
        pTrackMetadata->setComposer(toQString(tag.fieldListMap()["COMPOSER"]));
    }

    if (tag.fieldListMap().contains("GROUPING")) {
        pTrackMetadata->setGrouping(toQString(tag.fieldListMap()["GROUPING"]));
    }
}

void readMP4Tag(TrackMetadata* pTrackMetadata, /*const*/ TagLib::MP4::Tag& tag) {
    if (kDebugMetadata) {
        for(TagLib::MP4::ItemListMap::ConstIterator it = tag.itemListMap().begin();
            it != tag.itemListMap().end(); ++it) {
            qDebug() << "MP4" << toQString((*it).first) << "-"
                     << toQString((*it).second);
        }
    }

    readTag(pTrackMetadata, tag);

    // Get BPM
    if (tag.itemListMap().contains("tmpo")) {
        pTrackMetadata->setBpmString(toQString(tag.itemListMap()["tmpo"]));
    } else if (tag.itemListMap().contains("----:com.apple.iTunes:BPM")) {
        // This is an alternate way of storing BPM.
        pTrackMetadata->setBpmString(toQString(tag.itemListMap()["----:com.apple.iTunes:BPM"]));
    }

    // Get Album Artist
    if (tag.itemListMap().contains("aART")) {
        pTrackMetadata->setAlbumArtist(toQString(tag.itemListMap()["aART"]));
    }

    // Get Composer
    if (tag.itemListMap().contains("\251wrt")) {
        pTrackMetadata->setComposer(toQString(tag.itemListMap()["\251wrt"]));
    }

    // Get Grouping
    if (tag.itemListMap().contains("\251grp")) {
        pTrackMetadata->setGrouping(toQString(tag.itemListMap()["\251grp"]));
    }

    // Get KEY (conforms to Rapid Evolution)
    if (tag.itemListMap().contains("----:com.apple.iTunes:KEY")) {
        QString key = toQString(
            tag.itemListMap()["----:com.apple.iTunes:KEY"]);
        pTrackMetadata->setKey(key);
    }

    // Apparently iTunes stores replaygain in this property.
    if (tag.itemListMap().contains("----:com.apple.iTunes:replaygain_album_gain")) {
        // TODO(XXX) find tracks with this property and check what it looks
        // like.
        //QString replaygain = toQString(tag.itemListMap()["----:com.apple.iTunes:replaygain_album_gain"]);
    }
    //Prefer track gain over album gain.
    if (tag.itemListMap().contains("----:com.apple.iTunes:replaygain_track_gain")) {
        // TODO(XXX) find tracks with this property and check what it looks
        // like.
        //QString replaygain = toQString(tag.itemListMap()["----:com.apple.iTunes:replaygain_track_gain"]);
    }
}

QImage readID3v2TagCover(const TagLib::ID3v2::Tag& tag) {
    QImage coverArt;
    TagLib::ID3v2::FrameList covertArtFrame = tag.frameListMap()["APIC"];
    if (!covertArtFrame.isEmpty()) {
        TagLib::ID3v2::AttachedPictureFrame* picframe = static_cast
                <TagLib::ID3v2::AttachedPictureFrame*>(covertArtFrame.front());
        TagLib::ByteVector data = picframe->picture();
        coverArt = QImage::fromData(
            reinterpret_cast<const uchar *>(data.data()), data.size());
    }
    return coverArt;
}

QImage readAPETagCover(const TagLib::APE::Tag& tag) {
    QImage coverArt;
    if (tag.itemListMap().contains("COVER ART (FRONT)"))
    {
        const TagLib::ByteVector nullStringTerminator(1, 0);
        TagLib::ByteVector item = tag.itemListMap()["COVER ART (FRONT)"].value();
        int pos = item.find(nullStringTerminator);  // skip the filename
        if (++pos > 0) {
            const TagLib::ByteVector& data = item.mid(pos);
            coverArt = QImage::fromData(
                reinterpret_cast<const uchar *>(data.data()), data.size());
        }
    }
    return coverArt;
}

QImage readXiphCommentCover(const TagLib::Ogg::XiphComment& tag) {
    QImage coverArt;
    if (tag.fieldListMap().contains("METADATA_BLOCK_PICTURE")) {
        QByteArray data(QByteArray::fromBase64(
            tag.fieldListMap()["METADATA_BLOCK_PICTURE"].front().toCString()));
        TagLib::ByteVector tdata(data.data(), data.size());
        TagLib::FLAC::Picture p(tdata);
        data = QByteArray(p.data().data(), p.data().size());
        coverArt = QImage::fromData(data);
    } else if (tag.fieldListMap().contains("COVERART")) {
        QByteArray data(QByteArray::fromBase64(
            tag.fieldListMap()["COVERART"].toString().toCString()));
        coverArt = QImage::fromData(data);
    }
    return coverArt;
}

QImage readMP4TagCover(/*const*/ TagLib::MP4::Tag& tag) {
    QImage coverArt;
    if (tag.itemListMap().contains("covr")) {
        TagLib::MP4::CoverArtList coverArtList = tag.itemListMap()["covr"]
                                                        .toCoverArtList();
        TagLib::ByteVector data = coverArtList.front().data();
        coverArt = QImage::fromData(
            reinterpret_cast<const uchar *>(data.data()), data.size());
    }
    return coverArt;
}

bool writeTag(TagLib::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    pTag->setArtist(trackMetadata.getArtist().toStdString());
    pTag->setTitle(trackMetadata.getTitle().toStdString());
    pTag->setAlbum(trackMetadata.getAlbum().toStdString());
    pTag->setGenre(trackMetadata.getGenre().toStdString());
    pTag->setComment(trackMetadata.getComment().toStdString());
    uint year =  trackMetadata.getYear().toUInt();
    if (year >  0) {
        pTag->setYear(year);
    }
    uint track = trackMetadata.getTrackNumber().toUInt();
    if (track > 0) {
        pTag->setTrack(track);
    }

    return true;
}

bool writeID3v2Tag(TagLib::ID3v2::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    TagLib::ID3v2::FrameList albumArtistFrame = pTag->frameListMap()["TPE2"];
    if (!albumArtistFrame.isEmpty()) {
        albumArtistFrame.front()->setText(trackMetadata.getAlbumArtist().toStdString());
    } else {
        //add new frame
        TagLib::ID3v2::TextIdentificationFrame* newFrame =
                new TagLib::ID3v2::TextIdentificationFrame(
                    "TPE2", TagLib::String::Latin1);
        newFrame->setText(trackMetadata.getAlbumArtist().toStdString());
        pTag->addFrame(newFrame);
    }

    TagLib::ID3v2::FrameList bpmFrame = pTag->frameListMap()["TBPM"];
    if (!bpmFrame.isEmpty()) {
        bpmFrame.front()->setText(formatString(trackMetadata.getBpm()).toStdString());
    } else {
         // add new frame TextIdentificationFrame which is responsible for TKEY and TBPM
         // see http://developer.kde.org/~wheeler/taglib/api/classTagLib_1_1ID3v2_1_1TextIdentificationFrame.html

        TagLib::ID3v2::TextIdentificationFrame* newFrame = new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);

        newFrame->setText(formatString(trackMetadata.getBpm()).toStdString());
        pTag->addFrame(newFrame);

    }

    TagLib::ID3v2::FrameList keyFrame = pTag->frameListMap()["TKEY"];
    if (!keyFrame.isEmpty()) {
        keyFrame.front()->setText(trackMetadata.getKey().toStdString());
    } else {
        //add new frame
        TagLib::ID3v2::TextIdentificationFrame* newFrame = new TagLib::ID3v2::TextIdentificationFrame("TKEY", TagLib::String::Latin1);

        newFrame->setText(trackMetadata.getKey().toStdString());
        pTag->addFrame(newFrame);

    }

    TagLib::ID3v2::FrameList composerFrame = pTag->frameListMap()["TCOM"];
    if (!composerFrame.isEmpty()) {
        composerFrame.front()->setText(trackMetadata.getComposer().toStdString());
    } else {
        //add new frame
        TagLib::ID3v2::TextIdentificationFrame* newFrame =
                new TagLib::ID3v2::TextIdentificationFrame(
                    "TCOM", TagLib::String::Latin1);
        newFrame->setText(trackMetadata.getComposer().toStdString());
        pTag->addFrame(newFrame);
    }

    TagLib::ID3v2::FrameList groupingFrame = pTag->frameListMap()["TIT1"];
    if (!groupingFrame.isEmpty()) {
        groupingFrame.front()->setText(trackMetadata.getGrouping().toStdString());
    } else {
        //add new frame
        TagLib::ID3v2::TextIdentificationFrame* newFrame =
                new TagLib::ID3v2::TextIdentificationFrame(
                    "TIT1", TagLib::String::Latin1);
        newFrame->setText(trackMetadata.getGrouping().toStdString());
        pTag->addFrame(newFrame);
    }

    return true;
}

bool writeAPETag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    // Adds to the item specified by key the data value.
    // If replace is true, then all of the other values on the same key will be removed first.
    pTag->addValue("BPM", formatString(trackMetadata.getBpm()).toStdString(), true);
    pTag->addValue("Album Artist", trackMetadata.getAlbumArtist().toStdString(), true);
    pTag->addValue("Composer", trackMetadata.getComposer().toStdString(), true);
    pTag->addValue("Grouping", trackMetadata.getGrouping().toStdString(), true);

    return true;
}

bool writeXiphComment(TagLib::Ogg::XiphComment* pTag, const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    // Taglib does not support the update of Vorbis comments.
    // thus, we have to reomve the old comment and add the new one

    pTag->removeField("ALBUMARTIST");
    pTag->addField("ALBUMARTIST", trackMetadata.getAlbumArtist().toStdString());

    // Some tools use "BPM" so write that.
    pTag->removeField("BPM");
    pTag->addField("BPM",formatString(trackMetadata.getBpm()).toStdString());
    pTag->removeField("TEMPO");
    pTag->addField("TEMPO",formatString(trackMetadata.getBpm()).toStdString());

    pTag->removeField("INITIALKEY");
    pTag->addField("INITIALKEY", trackMetadata.getKey().toStdString());
    pTag->removeField("KEY");
    pTag->addField("KEY", trackMetadata.getKey().toStdString());

    pTag->removeField("COMPOSER");
    pTag->addField("COMPOSER", trackMetadata.getComposer().toStdString());

    pTag->removeField("GROUPING");
    pTag->addField("GROUPING", trackMetadata.getGrouping().toStdString());

    return true;
}

bool writeMP4Tag(TagLib::MP4::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    pTag->itemListMap()["aART"] = TagLib::StringList(trackMetadata.getAlbumArtist().toStdString());
    pTag->itemListMap()["tmpo"] = TagLib::StringList(formatString(trackMetadata.getBpm()).toStdString());
    pTag->itemListMap()["----:com.apple.iTunes:BPM"] = TagLib::StringList(formatString(trackMetadata.getBpm()).toStdString());
    pTag->itemListMap()["----:com.apple.iTunes:KEY"] = TagLib::StringList(trackMetadata.getKey().toStdString());
    pTag->itemListMap()["\251wrt"] = TagLib::StringList(trackMetadata.getComposer().toStdString());
    pTag->itemListMap()["\251grp"] = TagLib::StringList(trackMetadata.getGrouping().toStdString());

    return true;
}

} //namespace Mixxx
