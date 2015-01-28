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

#include <cmath>

#include <QDebug>

namespace Mixxx {

// static
namespace {
const bool kDebugMetadata = false;

// Taglib strings can be NULL and using it could cause some segfaults,
// so in this case it will return a QString()
inline QString toQString(const TagLib::String& tString) {
    if (tString.isNull()) {
        return QString();
    } else {
        return TStringToQString(tString);
    }
}

// Returns the first element of TagLib string list.
inline QString toQStringFirst(const TagLib::StringList& strList) {
    if (strList.isEmpty()) {
        return QString();
    } else {
        return toQString(strList.front());
    }
}

// Returns the text of an ID3v2 frame as a string.
inline QString toQString(const TagLib::ID3v2::Frame& frame) {
    return toQString(frame.toString());
}

// Returns the first frame of an ID3v2 tag as a string.
inline QString toQStringFirst(const TagLib::ID3v2::FrameList& frameList) {
    if (frameList.isEmpty() || (NULL == frameList.front())) {
        return QString();
    } else {
        return toQString(*frameList.front());
    }
}

// Returns the first value of an MP4 item as a string.
inline QString toQStringFirst(const TagLib::MP4::Item& mp4Item) {
    const TagLib::StringList strList(mp4Item.toStringList());
    if (strList.isEmpty()) {
        return QString();
    } else {
        return toQString(strList.front());
    }
}

// Returns an APE item as a string.
inline QString toQString(const TagLib::APE::Item& apeItem) {
    return toQString(apeItem.toString());
}

inline TagLib::String toTagLibString(const QString& str) {
    const QByteArray qba(str.toUtf8());
    return TagLib::String(qba.constData(), TagLib::String::UTF8);
}

} // anonymous namespace

bool readAudioProperties(TrackMetadata* pTrackMetadata,
        const TagLib::File& file) {
    if (kDebugMetadata) {
        qDebug() << "Parsing" << file.name();
    }

    if (file.isValid()) {
        const TagLib::AudioProperties *properties = file.audioProperties();
        if (properties) {
            pTrackMetadata->setChannels(properties->channels());
            pTrackMetadata->setSampleRate(properties->sampleRate());
            pTrackMetadata->setDuration(properties->length());
            pTrackMetadata->setBitrate(properties->bitrate());

            if (kDebugMetadata) {
                qDebug() << "TagLib" << "channels"
                        << pTrackMetadata->getChannels() << "sampleRate"
                        << pTrackMetadata->getSampleRate() << "bitrate"
                        << pTrackMetadata->getBitrate() << "duration"
                        << pTrackMetadata->getDuration();
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
        pTrackMetadata->setYear(QString::number(iYear));
    }

    int iTrack = tag.track();
    if (iTrack > 0) {
        pTrackMetadata->setTrackNumber(QString::number(iTrack));
    }

    if (kDebugMetadata) {
        qDebug() << "TagLib" << "title" << pTrackMetadata->getTitle()
                << "artist" << pTrackMetadata->getArtist() << "album"
                << pTrackMetadata->getAlbum() << "comment"
                << pTrackMetadata->getComment() << "genre"
                << pTrackMetadata->getGenre() << "year"
                << pTrackMetadata->getYear() << "trackNumber"
                << pTrackMetadata->getTrackNumber();
    }
}

void readID3v2Tag(TrackMetadata* pTrackMetadata,
        const TagLib::ID3v2::Tag& tag) {

    // Print every frame in the file.
    if (kDebugMetadata) {
        for (TagLib::ID3v2::FrameList::ConstIterator it(
                tag.frameList().begin()); it != tag.frameList().end(); ++it) {
            qDebug() << "ID3V2" << (*it)->frameID().data() << "-"
                    << toQString((*it)->toString());
        }
    }

    readTag(pTrackMetadata, tag);

    const TagLib::ID3v2::FrameList albumArtistFrame(tag.frameListMap()["TPE2"]);
    if (!albumArtistFrame.isEmpty()) {
        pTrackMetadata->setAlbumArtist(toQStringFirst(albumArtistFrame));
    }

    if (pTrackMetadata->getAlbum().isEmpty()) {
        const TagLib::ID3v2::FrameList originalAlbumFrame(
                tag.frameListMap()["TOAL"]);
        pTrackMetadata->setAlbum(toQStringFirst(originalAlbumFrame));
    }

    const TagLib::ID3v2::FrameList composerFrame(tag.frameListMap()["TCOM"]);
    if (!composerFrame.isEmpty()) {
        pTrackMetadata->setComposer(toQStringFirst(composerFrame));
    }

    const TagLib::ID3v2::FrameList groupingFrame(tag.frameListMap()["TIT1"]);
    if (!groupingFrame.isEmpty()) {
        pTrackMetadata->setGrouping(toQStringFirst(groupingFrame));
    }

    // ID3v2.4.0: TDRC replaces TYER + TDAT
    const TagLib::ID3v2::FrameList recordingDateFrame(
            tag.frameListMap()["TDRC"]);
    if (!recordingDateFrame.isEmpty()) {
        pTrackMetadata->setYear(toQStringFirst(recordingDateFrame));
    }

    const TagLib::ID3v2::FrameList bpmFrame(tag.frameListMap()["TBPM"]);
    if (!bpmFrame.isEmpty()) {
        pTrackMetadata->setBpmString(toQStringFirst(bpmFrame));
    }

    // Foobar2000-style ID3v2.3.0 tags
    // TODO: Check if everything is ok.
    TagLib::ID3v2::FrameList textFrames(tag.frameListMap()["TXXX"]);
    for (TagLib::ID3v2::FrameList::ConstIterator it(textFrames.begin());
            it != textFrames.end(); ++it) {
        TagLib::ID3v2::UserTextIdentificationFrame* replaygainFrame =
                dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
        if (replaygainFrame && replaygainFrame->fieldList().size() >= 2) {
            const QString desc(
                    toQString(replaygainFrame->description()));
            // Only read track gain (not album gain)
            if (0 == desc.compare("REPLAYGAIN_TRACK_GAIN", Qt::CaseInsensitive)) {
                pTrackMetadata->setReplayGainDbString(
                        toQString(replaygainFrame->fieldList()[1]));
            }
        }
    }

    const TagLib::ID3v2::FrameList keyFrame(tag.frameListMap()["TKEY"]);
    if (!keyFrame.isEmpty()) {
        pTrackMetadata->setKey(toQStringFirst(keyFrame));
    }
}

void readAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag) {
    if (kDebugMetadata) {
        for (TagLib::APE::ItemListMap::ConstIterator it(
                tag.itemListMap().begin()); it != tag.itemListMap().end();
                ++it) {
            qDebug() << "APE" << toQString((*it).first) << "-"
                    << toQString((*it).second.toString());
        }
    }

    readTag(pTrackMetadata, tag);

    if (tag.itemListMap().contains("Album Artist")) {
        pTrackMetadata->setAlbumArtist(
                toQString(tag.itemListMap()["Album Artist"]));
    }

    if (tag.itemListMap().contains("Composer")) {
        pTrackMetadata->setComposer(toQString(tag.itemListMap()["Composer"]));
    }

    if (tag.itemListMap().contains("Grouping")) {
        pTrackMetadata->setGrouping(toQString(tag.itemListMap()["Grouping"]));
    }

    if (tag.itemListMap().contains("Year")) {
        pTrackMetadata->setYear(toQString(tag.itemListMap()["Year"]));
    }

    if (tag.itemListMap().contains("BPM")) {
        pTrackMetadata->setBpmString(toQString(tag.itemListMap()["BPM"]));
    }

    // Only read track gain (not album gain)
    if (tag.itemListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
        pTrackMetadata->setReplayGainDbString(
                toQString(tag.itemListMap()["REPLAYGAIN_TRACK_GAIN"]));
    }
}

void readXiphComment(TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag) {
    if (kDebugMetadata) {
        for (TagLib::Ogg::FieldListMap::ConstIterator it(
                tag.fieldListMap().begin()); it != tag.fieldListMap().end();
                ++it) {
            qDebug() << "XIPH" << toQString((*it).first) << "-"
                    << toQString((*it).second.toString());
        }
    }

    readTag(pTrackMetadata, tag);

    // Some applications (like puddletag up to version 1.0.5) write COMMENT
    // instead DESCRIPTION. If the comment field (correctly populated by TagLib
    // from DESCRIPTION) is still empty we will additionally read this field.
    // Reference: http://www.xiph.org/vorbis/doc/v-comment.html
    if (pTrackMetadata->getComment().isEmpty()
            && tag.fieldListMap().contains("COMMENT")) {
        pTrackMetadata->setComment(
                toQStringFirst(tag.fieldListMap()["COMMENT"]));
    }

    if (tag.fieldListMap().contains("ALBUMARTIST")) {
        pTrackMetadata->setAlbumArtist(
                toQStringFirst(tag.fieldListMap()["ALBUMARTIST"]));
    }
    if (pTrackMetadata->getAlbumArtist().isEmpty()
            && tag.fieldListMap().contains("ALBUM_ARTIST")) {
        // try alternative field name
        pTrackMetadata->setAlbumArtist(
                toQStringFirst(tag.fieldListMap()["ALBUM_ARTIST"]));
    }
    if (pTrackMetadata->getAlbumArtist().isEmpty()
            && tag.fieldListMap().contains("ALBUM ARTIST")) {
        // try alternative field name
        pTrackMetadata->setAlbumArtist(
                toQStringFirst(tag.fieldListMap()["ALBUM ARTIST"]));
    }

    if (tag.fieldListMap().contains("COMPOSER")) {
        pTrackMetadata->setComposer(
                toQStringFirst(tag.fieldListMap()["COMPOSER"]));
    }

    if (tag.fieldListMap().contains("GROUPING")) {
        pTrackMetadata->setGrouping(
                toQStringFirst(tag.fieldListMap()["GROUPING"]));
    }

    if (tag.fieldListMap().contains("DATE")) {
        pTrackMetadata->setYear(toQStringFirst(tag.fieldListMap()["DATE"]));
    }

    // Some tags use "BPM" so check for that.
    if (tag.fieldListMap().contains("BPM")) {
        pTrackMetadata->setBpmString(toQStringFirst(tag.fieldListMap()["BPM"]));
    }

    // Give preference to the "TEMPO" tag which seems to be more standard
    if (tag.fieldListMap().contains("TEMPO")) {
        pTrackMetadata->setBpmString(
                toQStringFirst(tag.fieldListMap()["TEMPO"]));
    }

    // Only read track gain (not album gain)
    if (tag.fieldListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
        pTrackMetadata->setReplayGainDbString(
                toQStringFirst(tag.fieldListMap()["REPLAYGAIN_TRACK_GAIN"]));
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
        pTrackMetadata->setKey(toQStringFirst(tag.fieldListMap()["KEY"]));
    }
    if (tag.fieldListMap().contains("INITIALKEY")) {
        // This is the preferred field for storing the musical key.
        pTrackMetadata->setKey(
                toQStringFirst(tag.fieldListMap()["INITIALKEY"]));
    }
}

void readMP4Tag(TrackMetadata* pTrackMetadata, /*const*/TagLib::MP4::Tag& tag) {
    if (kDebugMetadata) {
        for (TagLib::MP4::ItemListMap::ConstIterator it(
                tag.itemListMap().begin()); it != tag.itemListMap().end();
                ++it) {
            qDebug() << "MP4" << toQString((*it).first) << "-"
                    << toQStringFirst((*it).second);
        }
    }

    readTag(pTrackMetadata, tag);

    // Get Album Artist
    if (tag.itemListMap().contains("aART")) {
        pTrackMetadata->setAlbumArtist(
                toQStringFirst(tag.itemListMap()["aART"]));
    }

    // Get Composer
    if (tag.itemListMap().contains("\251wrt")) {
        pTrackMetadata->setComposer(
                toQStringFirst(tag.itemListMap()["\251wrt"]));
    }

    // Get Grouping
    if (tag.itemListMap().contains("\251grp")) {
        pTrackMetadata->setGrouping(
                toQStringFirst(tag.itemListMap()["\251grp"]));
    }

    // Get date/year as string
    if (tag.itemListMap().contains("\251day")) {
        pTrackMetadata->setYear(toQStringFirst(tag.itemListMap()["\251day"]));
    }

    // Get BPM
    if (tag.itemListMap().contains("tmpo")) {
        // Read the BPM as an integer value.
        pTrackMetadata->setBpm(tag.itemListMap()["tmpo"].toInt());
    }
    if (tag.itemListMap().contains("----:com.apple.iTunes:BPM")) {
        // This is the preferred field for storing the BPM
        // with fractional digits as a floating-point value.
        // If this field contains a valid value the integer
        // BPM value that might have been read before is
        // overwritten.
        pTrackMetadata->setBpmString(
                toQStringFirst(tag.itemListMap()["----:com.apple.iTunes:BPM"]));
    }

    // Only read track gain (not album gain)
    if (tag.itemListMap().contains(
            "----:com.apple.iTunes:replaygain_track_gain")) {
        pTrackMetadata->setReplayGainDbString(
                toQStringFirst(tag.itemListMap()["----:com.apple.iTunes:replaygain_track_gain"]));
    }

    // Read musical key (conforms to Rapid Evolution)
    if (tag.itemListMap().contains("----:com.apple.iTunes:KEY")) {
        pTrackMetadata->setKey(
                toQStringFirst(tag.itemListMap()["----:com.apple.iTunes:KEY"]));
    }
    // Read musical key (conforms to MixedInKey, Serato, Traktor)
    if (tag.itemListMap().contains("----:com.apple.iTunes:initialkey")) {
        // This is the preferred field for storing the musical key!
        pTrackMetadata->setKey(
                toQStringFirst(tag.itemListMap()["----:com.apple.iTunes:initialkey"]));
    }
}

QImage readID3v2TagCover(const TagLib::ID3v2::Tag& tag) {
    QImage coverArt;
    TagLib::ID3v2::FrameList covertArtFrame = tag.frameListMap()["APIC"];
    if (!covertArtFrame.isEmpty()) {
        TagLib::ID3v2::AttachedPictureFrame* picframe =
                static_cast<TagLib::ID3v2::AttachedPictureFrame*>(covertArtFrame.front());
        TagLib::ByteVector data = picframe->picture();
        coverArt = QImage::fromData(
                reinterpret_cast<const uchar *>(data.data()), data.size());
    }
    return coverArt;
}

QImage readAPETagCover(const TagLib::APE::Tag& tag) {
    QImage coverArt;
    if (tag.itemListMap().contains("COVER ART (FRONT)")) {
        const TagLib::ByteVector nullStringTerminator(1, 0);
        TagLib::ByteVector item =
                tag.itemListMap()["COVER ART (FRONT)"].value();
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
        QByteArray data(
                QByteArray::fromBase64(
                        tag.fieldListMap()["METADATA_BLOCK_PICTURE"].front().toCString()));
        TagLib::ByteVector tdata(data.data(), data.size());
        TagLib::FLAC::Picture p(tdata);
        data = QByteArray(p.data().data(), p.data().size());
        coverArt = QImage::fromData(data);
    } else if (tag.fieldListMap().contains("COVERART")) {
        QByteArray data(
                QByteArray::fromBase64(
                        tag.fieldListMap()["COVERART"].toString().toCString()));
        coverArt = QImage::fromData(data);
    }
    return coverArt;
}

QImage readMP4TagCover(/*const*/TagLib::MP4::Tag& tag) {
    QImage coverArt;
    if (tag.itemListMap().contains("covr")) {
        TagLib::MP4::CoverArtList coverArtList =
                tag.itemListMap()["covr"].toCoverArtList();
        TagLib::ByteVector data = coverArtList.front().data();
        coverArt = QImage::fromData(
                reinterpret_cast<const uchar *>(data.data()), data.size());
    }
    return coverArt;
}

namespace {

void replaceID3v2Frame(TagLib::ID3v2::Tag* pTag, TagLib::ID3v2::Frame* pFrame) {
    pTag->removeFrames(pFrame->frameID());
    pTag->addFrame(pFrame);
}

void writeID3v2TextIdentificationFrame(TagLib::ID3v2::Tag* pTag,
        const TagLib::ByteVector &id, const QString& text) {
    TagLib::String::Type textType;
    QByteArray textData;
    if (4 <= pTag->header()->majorVersion()) {
        // prefer UTF-8 for ID3v2.4.0 or higher
        textType = TagLib::String::UTF8;
        textData = text.toUtf8();
    } else {
        textType = TagLib::String::Latin1;
        textData = text.toLatin1();
    }
    QScopedPointer<TagLib::ID3v2::TextIdentificationFrame> pNewFrame(
            new TagLib::ID3v2::TextIdentificationFrame(id, textType));
    pNewFrame->setText(toTagLibString(text));
    replaceID3v2Frame(pTag, pNewFrame.data());
    pNewFrame.take(); // release ownership
}

} // anonymous namespace

bool writeTag(TagLib::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    pTag->setArtist(toTagLibString(trackMetadata.getArtist()));
    pTag->setTitle(toTagLibString(trackMetadata.getTitle()));
    pTag->setAlbum(toTagLibString(trackMetadata.getAlbum()));
    pTag->setGenre(toTagLibString(trackMetadata.getGenre()));
    pTag->setComment(toTagLibString(trackMetadata.getComment()));
    bool yearValid = false;
    uint year = trackMetadata.getYear().toUInt(&yearValid);
    if (yearValid && (year > 0)) {
        pTag->setYear(year);
    }
    bool trackNumberValid = false;
    uint track = trackMetadata.getTrackNumber().toUInt(&trackNumberValid);
    if (trackNumberValid && (track > 0)) {
        pTag->setTrack(track);
    }

    return true;
}

bool writeID3v2Tag(TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }
    const TagLib::ID3v2::Header* pHeader = pTag->header();
    if ((NULL == pHeader) || (3 > pHeader->majorVersion())) {
        // only ID3v2.3.x and higher (currently only ID3v2.4.x) are supported
        return false;
    }

    // Write common metadata
    writeTag(pTag, trackMetadata);

    // additional tags
    writeID3v2TextIdentificationFrame(pTag, "TPE2",
            trackMetadata.getAlbumArtist());
    writeID3v2TextIdentificationFrame(pTag, "TBPM",
            trackMetadata.getBpmString());
    writeID3v2TextIdentificationFrame(pTag, "TBPM",
            trackMetadata.getBpmString());
    writeID3v2TextIdentificationFrame(pTag, "TKEY", trackMetadata.getKey());
    writeID3v2TextIdentificationFrame(pTag, "TCOM",
            trackMetadata.getComposer());
    writeID3v2TextIdentificationFrame(pTag, "TIT1",
            trackMetadata.getGrouping());
    if (4 <= pHeader->majorVersion()) {
        // ID3v2.4.0: TDRC replaces TYER + TDAT
        writeID3v2TextIdentificationFrame(pTag, "TDRC",
                trackMetadata.getYear());
    }
    // TODO(uklotzde): Write TXXX - REPLAYGAIN_TRACK_GAIN

    return true;
}

bool writeAPETag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    // Write common metadata
    writeTag(pTag, trackMetadata);

    pTag->addValue("Album Artist",
            toTagLibString(trackMetadata.getAlbumArtist()), true);
    pTag->addValue("Composer",
            toTagLibString(trackMetadata.getComposer()), true);
    pTag->addValue("Grouping",
            toTagLibString(trackMetadata.getGrouping()), true);
    pTag->addValue("Year",
            toTagLibString(trackMetadata.getYear()), true);
    pTag->addValue("BPM",
            toTagLibString(trackMetadata.getBpmString()), true);
    pTag->addValue("REPLAYGAIN_TRACK_GAIN",
            toTagLibString(trackMetadata.getReplayGainDbString()), true);

    return true;
}

bool writeXiphComment(TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    // Write common metadata
    writeTag(pTag, trackMetadata);

    // Taglib does not support the update of Vorbis comments.
    // thus, we have to remove the old comment and add the new one

    pTag->removeField("ALBUMARTIST");
    pTag->addField("ALBUMARTIST",
            toTagLibString(trackMetadata.getAlbumArtist()));

    pTag->removeField("COMPOSER");
    pTag->addField("COMPOSER", toTagLibString(trackMetadata.getComposer()));

    pTag->removeField("GROUPING");
    pTag->addField("GROUPING", toTagLibString(trackMetadata.getGrouping()));

    pTag->removeField("DATE");
    pTag->addField("DATE", toTagLibString(trackMetadata.getYear()));

    // Some tools use "BPM" so write that.
    pTag->removeField("BPM");
    pTag->addField("BPM",
            toTagLibString(trackMetadata.getBpmString()));
    pTag->removeField("TEMPO");
    pTag->addField("TEMPO",
            toTagLibString(trackMetadata.getBpmString()));

    pTag->removeField("INITIALKEY");
    pTag->addField("INITIALKEY", toTagLibString(trackMetadata.getKey()));
    pTag->removeField("KEY");
    pTag->addField("KEY", toTagLibString(trackMetadata.getKey()));

    pTag->removeField("REPLAYGAIN_TRACK_GAIN");
    pTag->addField("REPLAYGAIN_TRACK_GAIN",
            toTagLibString(trackMetadata.getReplayGainDbString()));

    return true;
}

namespace {

template<typename T>
inline void writeMP4Atom(TagLib::MP4::Tag* pTag, const TagLib::String& key,
        const T& value) {
    pTag->itemListMap()[key] = value;
}

void writeMP4Atom(TagLib::MP4::Tag* pTag, const TagLib::String& key,
        const QString& value) {
    if (value.isEmpty()) {
        pTag->itemListMap().erase(key);
    } else {
        writeMP4Atom(pTag, key, TagLib::StringList(toTagLibString(value)));
    }
}

} // anonymous namespace

bool writeMP4Tag(TagLib::MP4::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (NULL == pTag) {
        return false;
    }

    // Write common metadata
    writeTag(pTag, trackMetadata);

    writeMP4Atom(pTag, "aART", trackMetadata.getAlbumArtist());
    writeMP4Atom(pTag, "\251wrt", trackMetadata.getComposer());
    writeMP4Atom(pTag, "\251grp", trackMetadata.getGrouping());
    writeMP4Atom(pTag, "\251day", trackMetadata.getYear());
    if (trackMetadata.isBpmValid()) {
        writeMP4Atom(pTag, "tmpo", (int) round(trackMetadata.getBpm()));
    } else {
        pTag->itemListMap().erase("tmpo");
    }
    writeMP4Atom(pTag, "----:com.apple.iTunes:BPM",
            trackMetadata.getBpmString());
    writeMP4Atom(pTag, "----:com.apple.iTunes:replaygain_track_gain",
            trackMetadata.getReplayGainDbString());
    writeMP4Atom(pTag, "----:com.apple.iTunes:initialkey",
            trackMetadata.getKey());
    writeMP4Atom(pTag, "----:com.apple.iTunes:KEY",
            trackMetadata.getKey());

    return true;
}

} //namespace Mixxx
