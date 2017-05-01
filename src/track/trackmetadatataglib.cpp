#include <array>

#include "track/trackmetadatataglib.h"

#include "track/tracknumbers.h"

#include "util/assert.h"
#include "util/duration.h"
#include "util/memory.h"

// TagLib has full support for MP4 atom types since version 1.8
#define TAGLIB_HAS_MP4_ATOM_TYPES \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 8))

// TagLib has support for the Ogg Opus file format since version 1.9
#define TAGLIB_HAS_OPUSFILE \
    ((TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9)))

// TagLib has support for hasID3v2Tag()/ID3v2Tag() for WAV files since version 1.9
#define TAGLIB_HAS_WAV_ID3V2TAG \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))

// TagLib has support for has<TagType>() style functions since version 1.9
#define TAGLIB_HAS_TAG_CHECK \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))

// TagLib has support for hasID3v2Tag() for AIFF files since version 1.10
#define TAGLIB_HAS_AIFF_HAS_ID3V2TAG \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 10))

// TagLib has support for length in milliseconds since version 1.10
#define TAGLIB_HAS_LENGTH_IN_MILLISECONDS \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 10))

// TagLib has support for XiphComment::pictureList() since version 1.11
#define TAGLIB_HAS_VORBIS_COMMENT_PICTURES \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 11))

#include <taglib/tfile.h>
#include <taglib/tmap.h>
#include <taglib/tstringlist.h>

#include <taglib/mp4file.h>
#include <taglib/vorbisfile.h>
#if (TAGLIB_HAS_OPUSFILE)
#include <taglib/opusfile.h>
#endif
#include <taglib/wavfile.h>
#include <taglib/aifffile.h>

#include <taglib/commentsframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacpicture.h>

namespace mixxx {

namespace taglib {

QDebug operator<<(QDebug debug, FileType fileType) {
    return debug << static_cast<std::underlying_type<FileType>::type>(fileType);
}

bool hasID3v1Tag(TagLib::MPEG::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasID3v1Tag();
#else
    return nullptr != file.ID3v1Tag();
#endif
}

bool hasID3v2Tag(TagLib::MPEG::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasID3v2Tag();
#else
    return nullptr != file.ID3v2Tag();
#endif
}

bool hasAPETag(TagLib::MPEG::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasAPETag();
#else
    return nullptr != file.APETag();
#endif
}

bool hasID3v2Tag(TagLib::FLAC::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasID3v2Tag();
#else
    return nullptr != file.ID3v2Tag();
#endif
}

bool hasXiphComment(TagLib::FLAC::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasXiphComment();
#else
    return nullptr != file.xiphComment();
#endif
}

bool hasAPETag(TagLib::WavPack::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasAPETag();
#else
    return nullptr != file.APETag();
#endif
}

namespace {

// Preferred picture types for cover art sorted by priority
const std::array<TagLib::ID3v2::AttachedPictureFrame::Type, 4> kPreferredID3v2PictureTypes = {{
        TagLib::ID3v2::AttachedPictureFrame::FrontCover, // Front cover image of the album
        TagLib::ID3v2::AttachedPictureFrame::Media, // Image from the album itself
        TagLib::ID3v2::AttachedPictureFrame::Illustration, // Illustration related to the track
        TagLib::ID3v2::AttachedPictureFrame::Other
}};
const std::array<TagLib::FLAC::Picture::Type, 4> kPreferredVorbisCommentPictureTypes = {{
        TagLib::FLAC::Picture::FrontCover, // Front cover image of the album
        TagLib::FLAC::Picture::Media, // Image from the album itself
        TagLib::FLAC::Picture::Illustration, // Illustration related to the track
        TagLib::FLAC::Picture::Other
}};

// Deduce the file type from the file name
FileType getFileTypeFromFileName(QString fileName) {
    DEBUG_ASSERT(!fileName.isEmpty());
    const QString fileExt(fileName.section(".", -1).toLower().trimmed());
    if ("mp3" == fileExt) {
        return FileType::MP3;
    }
    if ("m4a" == fileExt) {
        return FileType::MP4;
    }
    if ("flac" == fileExt) {
        return FileType::FLAC;
    }
    if ("ogg" == fileExt) {
        return FileType::OGG;
    }
    if ("opus" == fileExt) {
        return FileType::OPUS;
    }
    if ("wav" == fileExt) {
        return FileType::WAV;
    }
    if ("wv" == fileExt) {
        return FileType::WV;
    }
    if (fileExt.startsWith("aif")) {
        return FileType::AIFF;
    }
    return FileType::UNKNOWN;
}

// http://id3.org/id3v2.3.0
// "TYER: The 'Year' frame is a numeric string with a year of the
// recording. This frame is always four characters long (until
// the year 10000)."
const QString ID3V2_TYER_FORMAT("yyyy");

// http://id3.org/id3v2.3.0
// "TDAT:  The 'Date' frame is a numeric string in the DDMM
// format containing the date for the recording. This field
// is always four characters long."
const QString ID3V2_TDAT_FORMAT("ddMM");

// Taglib strings can be nullptr and using it could cause some segfaults,
// so in this case it will return a QString()
inline QString toQString(const TagLib::String& tString) {
    if (tString.isNull()) {
        return QString();
    } else {
        return TStringToQString(tString);
    }
}

// Returns the first element of TagLib string list that is not empty.
QString toQStringFirstNotEmpty(const TagLib::StringList& strList) {
    for (const auto& str: strList) {
        if (!str.isEmpty()) {
            return toQString(str);
        }
    }
    return QString();
}

// Returns the text of an ID3v2 frame as a string.
inline QString toQString(const TagLib::ID3v2::Frame& frame) {
    return toQString(frame.toString());
}

// Returns the first frame of an ID3v2 tag as a string.
QString toQStringFirstNotEmpty(
        const TagLib::ID3v2::FrameList& frameList) {
    for (const TagLib::ID3v2::Frame* pFrame: frameList) {
        if (nullptr != pFrame) {
            TagLib::String str(pFrame->toString());
            if (!str.isEmpty()) {
                return toQString(str);
            }
        }
    }
    return QString();
}

// Returns the first non-empty value of an MP4 item as a string.
inline QString toQStringFirstNotEmpty(const TagLib::MP4::Item& mp4Item) {
    return toQStringFirstNotEmpty(mp4Item.toStringList());
}

inline TagLib::String toTagLibString(const QString& str) {
    const QByteArray qba(str.toUtf8());
    return TagLib::String(qba.constData(), TagLib::String::UTF8);
}

inline QString formatBpm(const TrackMetadata& trackMetadata) {
    return Bpm::valueToString(trackMetadata.getBpm().getValue());
}

inline QString formatBpmInteger(const TrackMetadata& trackMetadata) {
    return QString::number(Bpm::valueToInteger(trackMetadata.getBpm().getValue()));
}

bool parseBpm(TrackMetadata* pTrackMetadata, QString sBpm) {
    DEBUG_ASSERT(pTrackMetadata);

    bool isBpmValid = false;
    const double bpmValue = Bpm::valueFromString(sBpm, &isBpmValid);
    if (isBpmValid) {
        pTrackMetadata->setBpm(Bpm(bpmValue));
    }
    return isBpmValid;
}

inline QString formatTrackGain(const TrackMetadata& trackMetadata) {
    const double trackGainRatio(trackMetadata.getReplayGain().getRatio());
    return ReplayGain::ratioToString(trackGainRatio);
}

bool parseTrackGain(
        TrackMetadata* pTrackMetadata,
        const QString& dbGain) {
    DEBUG_ASSERT(pTrackMetadata);

    bool isRatioValid = false;
    double ratio = ReplayGain::ratioFromString(dbGain, &isRatioValid);
    if (isRatioValid) {
        // Some applications (e.g. Rapid Evolution 3) write a replay gain
        // of 0 dB even if the replay gain is undefined. To be safe we
        // ignore this special value and instead prefer to recalculate
        // the replay gain.
        if (ratio == ReplayGain::kRatio0dB) {
            // special case
            qDebug() << "Ignoring possibly undefined gain:" << dbGain;
            ratio = ReplayGain::kRatioUndefined;
        }
        ReplayGain replayGain(pTrackMetadata->getReplayGain());
        replayGain.setRatio(ratio);
        pTrackMetadata->setReplayGain(replayGain);
    }
    return isRatioValid;
}

inline QString formatTrackPeak(const TrackMetadata& trackMetadata) {
    const CSAMPLE trackGainPeak(trackMetadata.getReplayGain().getPeak());
    return ReplayGain::peakToString(trackGainPeak);
}

bool parseTrackPeak(
        TrackMetadata* pTrackMetadata,
        const QString& strPeak) {
    DEBUG_ASSERT(pTrackMetadata);

    bool isPeakValid = false;
    const CSAMPLE peak = ReplayGain::peakFromString(strPeak, &isPeakValid);
    if (isPeakValid) {
        ReplayGain replayGain(pTrackMetadata->getReplayGain());
        replayGain.setPeak(peak);
        pTrackMetadata->setReplayGain(replayGain);
    }
    return isPeakValid;
}

void readAudioProperties(TrackMetadata* pTrackMetadata,
        const TagLib::AudioProperties& audioProperties) {
    DEBUG_ASSERT(pTrackMetadata);

    // NOTE(uklotzde): All audio properties will be updated
    // with the actual (and more precise) values when reading
    // the audio data for this track. Often those properties
    // stored in tags don't match with the corresponding
    // audio data in the file.
    pTrackMetadata->setChannels(audioProperties.channels());
    pTrackMetadata->setSampleRate(audioProperties.sampleRate());
    pTrackMetadata->setBitrate(audioProperties.bitrate());
#if (TAGLIB_HAS_LENGTH_IN_MILLISECONDS)
    // Cast to double is required for duration with sub-second precision
    const double dLengthInMilliseconds = audioProperties.lengthInMilliseconds();
    const double duration = dLengthInMilliseconds / mixxx::Duration::kMillisPerSecond;
#else
    const double duration = audioProperties.length();
#endif
    pTrackMetadata->setDuration(duration);
}

bool readAudioProperties(TrackMetadata* pTrackMetadata,
        const TagLib::File& file) {
    if (!file.isValid()) {
        return false;
    }
    if (!pTrackMetadata) {
        // implicitly successful
        return true;
    }
    const TagLib::AudioProperties* pAudioProperties =
            file.audioProperties();
    if (!pAudioProperties) {
        qWarning() << "Failed to read audio properties from file"
                << file.name();
        return false;
    }
    readAudioProperties(pTrackMetadata, *pAudioProperties);
    return true;
}

void readTrackMetadataFromRIFFTag(TrackMetadata* pTrackMetadata, const TagLib::RIFF::Info::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

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
}

void readTrackMetadataFromTag(TrackMetadata* pTrackMetadata, const TagLib::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

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
}

// Workaround for missing const member function in TagLib
inline const TagLib::MP4::ItemListMap& getItemListMap(const TagLib::MP4::Tag& tag) {
    return const_cast<TagLib::MP4::Tag&>(tag).itemListMap();
}

inline QImage loadImageFromByteVector(
        const TagLib::ByteVector& imageData,
        const char* format = 0) {
    return QImage::fromData(
            // char -> uchar
            reinterpret_cast<const uchar *>(imageData.data()),
            imageData.size(),
            format);
}

inline QImage loadImageFromID3v2PictureFrame(
        const TagLib::ID3v2::AttachedPictureFrame& apicFrame) {
    return loadImageFromByteVector(apicFrame.picture());
}

inline QImage loadImageFromVorbisCommentPicture(
        const TagLib::FLAC::Picture& picture) {
    return loadImageFromByteVector(picture.data(), picture.mimeType().toCString());
}

QImage loadCoverArtImageFromVorbisCommentPictureList(
        const TagLib::List<TagLib::FLAC::Picture*>& pictures) {
    if (pictures.isEmpty()) {
        qDebug() << "VorbisComment picture list is empty";
        return QImage();
    }

    for (const auto coverArtType: kPreferredVorbisCommentPictureTypes) {
        for (const auto pPicture: pictures) {
            DEBUG_ASSERT(pPicture != nullptr); // trust TagLib
            if (pPicture->type() == coverArtType) {
                const QImage image(loadImageFromVorbisCommentPicture(*pPicture));
                if (image.isNull()) {
                    qWarning() << "Failed to load image from VorbisComment picture of type" << pPicture->type();
                    // continue...
                } else {
                    return image; // success
                }
            }
        }
    }

    // Fallback: No best match -> Create image from first loadable picture of any type
    for (const auto pPicture: pictures) {
        DEBUG_ASSERT(pPicture != nullptr); // trust TagLib
        const QImage image(loadImageFromVorbisCommentPicture(*pPicture));
        if (image.isNull()) {
            qWarning() << "Failed to load image from VorbisComment picture of type" << pPicture->type();
            // continue...
        } else {
            return image; // success
        }
    }

    qWarning() << "Failed to load cover art image from VorbisComment pictures";
    return QImage();
}

bool parseBase64EncodedVorbisCommentPicture(
        TagLib::FLAC::Picture* pPicture,
        const TagLib::String& base64Encoded) {
    DEBUG_ASSERT(pPicture != nullptr);
    const QByteArray decodedData(QByteArray::fromBase64(base64Encoded.toCString()));
    const TagLib::ByteVector rawData(decodedData.data(), decodedData.size());
    TagLib::FLAC::Picture picture;
    return pPicture->parse(rawData);
}

inline QImage parseBase64EncodedVorbisCommentImage(
        const TagLib::String& base64Encoded) {
    const QByteArray decodedData(QByteArray::fromBase64(base64Encoded.toCString()));
    return QImage::fromData(decodedData);
}

void readCoverArtFromID3v2Tag(QImage* pCoverArt, const TagLib::ID3v2::Tag& tag) {
    if (pCoverArt == nullptr) {
        return; // nothing to do
    }

    TagLib::ID3v2::FrameList pFrames = tag.frameListMap()["APIC"];
    if (pFrames.isEmpty()) {
        qDebug() << "Failed to load cover art: Empty list of ID3v2 APIC frames";
        return; // failure
    }

    for (const auto coverArtType: kPreferredID3v2PictureTypes) {
        for (const auto pFrame: pFrames) {
            const TagLib::ID3v2::AttachedPictureFrame* pApicFrame =
                    static_cast<const TagLib::ID3v2::AttachedPictureFrame*>(pFrame);
            DEBUG_ASSERT(pApicFrame != nullptr); // trust TagLib
            if (pApicFrame->type() == coverArtType) {
                QImage image(loadImageFromID3v2PictureFrame(*pApicFrame));
                if (image.isNull()) {
                    qWarning() << "Failed to load image from ID3v2 APIC frame of type" << pApicFrame->type();
                    // continue...
                } else {
                    *pCoverArt = image;
                    return; // success
                }
            }
        }
    }

    // Fallback: No best match -> Simply select the 1st loadable image
    for (const auto pFrame: pFrames) {
        const TagLib::ID3v2::AttachedPictureFrame* pApicFrame =
                static_cast<const TagLib::ID3v2::AttachedPictureFrame*>(pFrame);
        DEBUG_ASSERT(pApicFrame != nullptr); // trust TagLib
        const QImage image(loadImageFromID3v2PictureFrame(*pApicFrame));
        if (image.isNull()) {
            qWarning() << "Failed to load image from ID3v2 APIC frame of type" << pApicFrame->type();
            // continue...
        } else {
            *pCoverArt = image;
            return; // success
        }
    }
}

void readCoverArtFromAPETag(QImage* pCoverArt, const TagLib::APE::Tag& tag) {
    if (pCoverArt == nullptr) {
        return; // nothing to do
    }

    if (tag.itemListMap().contains("COVER ART (FRONT)")) {
        const TagLib::ByteVector nullStringTerminator(1, 0);
        TagLib::ByteVector item =
                tag.itemListMap()["COVER ART (FRONT)"].value();
        int pos = item.find(nullStringTerminator);  // skip the filename
        if (++pos > 0) {
            const TagLib::ByteVector data(item.mid(pos));
            const QImage image(loadImageFromByteVector(data));
            if (image.isNull()) {
                qWarning() << "Failed to load image from APE tag";
            } else {
                *pCoverArt = image; // success
            }
        }
    }
}

void readCoverArtFromVorbisCommentTag(QImage* pCoverArt, TagLib::Ogg::XiphComment& tag) {
    if (pCoverArt == nullptr) {
        return; // nothing to do
    }

#if (TAGLIB_HAS_VORBIS_COMMENT_PICTURES)
    const QImage image(loadCoverArtImageFromVorbisCommentPictureList(tag.pictureList()));
    if (!image.isNull()) {
        *pCoverArt = image;
        return; // done
    }
#endif

    // NOTE(uklotzde, 2016-07-13): Legacy code for parsing cover art (part 1)
    //
    // The following code is needed for TagLib versions <= 1.10 and as a workaround
    // for an incompatibility between TagLib 1.11 and puddletag 1.1.1.
    //
    // puddletag 1.1.1 seems to generate an incompatible METADATA_BLOCK_PICTURE
    // field that is not recognized by XiphComment::pictureList() by TagLib 1.11.
    // In this case XiphComment::pictureList() returns an empty list while the
    // raw data of the pictures can still be found in XiphComment::fieldListMap().
    if (tag.fieldListMap().contains("METADATA_BLOCK_PICTURE")) {
        // https://wiki.xiph.org/VorbisComment#METADATA_BLOCK_PICTURE
        const TagLib::StringList& base64EncodedList =
                tag.fieldListMap()["METADATA_BLOCK_PICTURE"];
#if (TAGLIB_HAS_VORBIS_COMMENT_PICTURES)
        if (!base64EncodedList.isEmpty()) {
            qWarning() << "Taking legacy code path for reading cover art from VorbisComment field METADATA_BLOCK_PICTURE";
        }
#endif
        for (const auto& base64Encoded: base64EncodedList) {
            TagLib::FLAC::Picture picture;
            if (parseBase64EncodedVorbisCommentPicture(&picture, base64Encoded)) {
                const QImage image(loadImageFromVorbisCommentPicture(picture));
                if (image.isNull()) {
                    qWarning() << "Failed to load image from VorbisComment picture of type" << picture.type();
                    // continue...
                } else {
                    *pCoverArt = image;
                    return; // done
                }
            } else {
                qWarning() << "Failed to parse picture from VorbisComment metadata block";
                // continue...
            }
        }
    }

    // NOTE(uklotzde, 2016-07-13): Legacy code for parsing cover art (part 2)
    //
    // The unofficial COVERART field in a VorbisComment tag is deprecated:
    // https://wiki.xiph.org/VorbisComment#Unofficial_COVERART_field_.28deprecated.29
    if (tag.fieldListMap().contains("COVERART")) {
        const TagLib::StringList& base64EncodedList =
                tag.fieldListMap()["COVERART"];
        if (!base64EncodedList.isEmpty()) {
            qWarning() << "Fallback: Trying to parse image from deprecated VorbisComment field COVERART";
        }
        for (const auto& base64Encoded: base64EncodedList) {
            const QImage image(parseBase64EncodedVorbisCommentImage(base64Encoded));
            if (image.isNull()) {
                qWarning() << "Failed to parse image from deprecated VorbisComment field COVERART";
                // continue...
            } else {
                *pCoverArt = image;
                return; // done
            }
        }
    }

    qDebug() << "No cover art found in VorbisComment tag";
}

void readCoverArtFromMP4Tag(QImage* pCoverArt, const TagLib::MP4::Tag& tag) {
    if (pCoverArt == nullptr) {
        return; // nothing to do
    }

    if (getItemListMap(tag).contains("covr")) {
        TagLib::MP4::CoverArtList coverArtList =
                getItemListMap(tag)["covr"].toCoverArtList();
        for (const auto& coverArt: coverArtList) {
            const QImage image(loadImageFromByteVector(coverArt.data()));
            if (image.isNull()) {
                qWarning() << "Failed to load image from MP4 atom covr";
                // continue...
            } else {
                *pCoverArt = image;
                return; // done
            }
        }
    }
}

TagLib::String::Type getID3v2StringType(const TagLib::ID3v2::Tag& tag, bool isNumericOrURL = false) {
    TagLib::String::Type stringType;
    // For an overview of the character encodings supported by
    // the different ID3v2 versions please refer to the following
    // resources:
    // http://en.wikipedia.org/wiki/ID3#ID3v2
    // http://id3.org/id3v2.3.0
    // http://id3.org/id3v2.4.0-structure
    if (4 <= tag.header()->majorVersion()) {
        // For ID3v2.4.0 or higher prefer UTF-8, because it is a
        // very compact representation for common cases and it is
        // independent of the byte order.
        stringType = TagLib::String::UTF8;
    } else {
        if (isNumericOrURL) {
            // According to the ID3v2.3.0 specification: "All numeric
            // strings and URLs are always encoded as ISO-8859-1."
            stringType = TagLib::String::Latin1;
        } else {
            // For ID3v2.3.0 use UCS-2 (UTF-16 encoded Unicode with BOM)
            // for arbitrary text, because UTF-8 and UTF-16BE are only
            // supported since ID3v2.4.0 and the alternative ISO-8859-1
            // does not cover all Unicode characters.
            stringType = TagLib::String::UTF16;
        }
    }
    return stringType;
}

// Finds the first comments frame with a matching description.
// If multiple comments frames with matching descriptions exist
// prefer the first with a non-empty content if requested.
TagLib::ID3v2::CommentsFrame* findFirstCommentsFrame(
        const TagLib::ID3v2::Tag& tag,
        const QString& description = QString(),
        bool preferNotEmpty = true) {
    TagLib::ID3v2::CommentsFrame* pFirstFrame = nullptr;
    // Bind the const-ref result to avoid a local copy
    const TagLib::ID3v2::FrameList& commentsFrames =
            tag.frameListMap()["COMM"];
    for (TagLib::ID3v2::FrameList::ConstIterator it(commentsFrames.begin());
            it != commentsFrames.end(); ++it) {
        auto pFrame =
                dynamic_cast<TagLib::ID3v2::CommentsFrame*>(*it);
        if (nullptr != pFrame) {
            const QString frameDescription(
                    toQString(pFrame->description()));
            if (0 == frameDescription.compare(
                    description, Qt::CaseInsensitive)) {
                if (preferNotEmpty && pFrame->toString().isEmpty()) {
                    // we might need the first matching frame later
                    // even if it is empty
                    if (pFirstFrame == nullptr) {
                        pFirstFrame = pFrame;
                    }
                } else {
                    // found what we are looking for
                    return pFrame;
                }
            }
        }
    }
    // simply return the first matching frame
    return pFirstFrame;
}

// Finds the first text frame that with a matching description (case-insensitive).
// If multiple comments frames with matching descriptions exist prefer the first
// with a non-empty content if requested.
TagLib::ID3v2::UserTextIdentificationFrame* findFirstUserTextIdentificationFrame(
        const TagLib::ID3v2::Tag& tag,
        const QString& description,
        bool preferNotEmpty = true) {
    DEBUG_ASSERT(!description.isEmpty());
    TagLib::ID3v2::UserTextIdentificationFrame* pFirstFrame = nullptr;
    // Bind the const-ref result to avoid a local copy
    const TagLib::ID3v2::FrameList& textFrames =
            tag.frameListMap()["TXXX"];
    for (TagLib::ID3v2::FrameList::ConstIterator it(textFrames.begin());
            it != textFrames.end(); ++it) {
        auto pFrame =
                dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
        if (nullptr != pFrame) {
            const QString frameDescription(
                    toQString(pFrame->description()));
            if (0 == frameDescription.compare(
                    description, Qt::CaseInsensitive)) {
                if (preferNotEmpty && pFrame->toString().isEmpty()) {
                    // we might need the first matching frame later
                    // even if it is empty
                    if (pFirstFrame == nullptr) {
                        pFirstFrame = pFrame;
                    }
                } else {
                    // found what we are looking for
                    return pFrame;
                }
            }
        }
    }
    // simply return the first matching frame
    return pFirstFrame;
}

// Deletes all TXXX frame with the given description (case-insensitive).
int removeUserTextIdentificationFrames(
        TagLib::ID3v2::Tag* pTag,
        const QString& description) {
    DEBUG_ASSERT(pTag != nullptr);
    DEBUG_ASSERT(!description.isEmpty());
    int count = 0;
    bool repeat;
    do {
        repeat = false;
        // Bind the const-ref result to avoid a local copy
        const TagLib::ID3v2::FrameList& textFrames =
                pTag->frameListMap()["TXXX"];
        for (TagLib::ID3v2::FrameList::ConstIterator it(textFrames.begin());
                it != textFrames.end(); ++it) {
            auto pFrame =
                    dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
            if (pFrame != nullptr) {
                const QString frameDescription(
                        toQString(pFrame->description()));
                if (0 == frameDescription.compare(
                        description, Qt::CaseInsensitive)) {
                    qDebug() << "Removing ID3v2 TXXX frame:" << toQString(pFrame->description());
                    // After removing a frame the result of frameListMap()
                    // is no longer valid!!
                    pTag->removeFrame(pFrame, false); // remove an unowned frame
                    ++count;
                    // Exit and restart loop
                    repeat = true;
                    break;
                }
            }
        }
    } while (repeat);
    return count;
}

void writeID3v2TextIdentificationFrame(
        TagLib::ID3v2::Tag* pTag,
        const TagLib::ByteVector &id,
        const QString& text,
        bool isNumericOrURL = false) {
    DEBUG_ASSERT(pTag);

    // Remove all existing frames before adding a new one
    pTag->removeFrames(id);
    if (!text.isEmpty()) {
        // Only add non-empty frames
        const TagLib::String::Type stringType =
                getID3v2StringType(*pTag, isNumericOrURL);
        auto pFrame =
                std::make_unique<TagLib::ID3v2::TextIdentificationFrame>(id, stringType);
        pFrame->setText(toTagLibString(text));
        pTag->addFrame(pFrame.get());
        // Now that the plain pointer in pFrame is owned and managed by
        // pTag we need to release the ownership to avoid double deletion!
        pFrame.release();
    }
}

void writeID3v2CommentsFrame(
        TagLib::ID3v2::Tag* pTag,
        const QString& text,
        const QString& description = QString(),
        bool isNumericOrURL = false) {
    TagLib::ID3v2::CommentsFrame* pFrame =
            findFirstCommentsFrame(*pTag, description);
    if (nullptr != pFrame) {
        // Modify existing frame
        if (text.isEmpty()) {
            // Purge empty frames
            pTag->removeFrame(pFrame);
        } else {
            pFrame->setDescription(toTagLibString(description));
            pFrame->setText(toTagLibString(text));
        }
    } else {
        // Add a new (non-empty) frame
        if (!text.isEmpty()) {
            const TagLib::String::Type stringType =
                    getID3v2StringType(*pTag, isNumericOrURL);
            auto pFrame =
                    std::make_unique<TagLib::ID3v2::CommentsFrame>(stringType);
            pFrame->setDescription(toTagLibString(description));
            pFrame->setText(toTagLibString(text));
            pTag->addFrame(pFrame.get());
            // Now that the plain pointer in pFrame is owned and managed by
            // pTag we need to release the ownership to avoid double deletion!
            pFrame.release();
        }
    }
    // Cleanup: Remove non-standard comment frames to avoid redundant and
    // inconsistent tags.
    // See also: Compatibility workaround when reading ID3v2 comment tags.
    int numberOfRemovedCommentFrames =
            removeUserTextIdentificationFrames(pTag, "COMMENT");
    if (numberOfRemovedCommentFrames > 0) {
        qWarning() << "Removed" << numberOfRemovedCommentFrames
                << "non-standard ID3v2 TXXX comment frames";
    }
}

void writeID3v2UserTextIdentificationFrame(
        TagLib::ID3v2::Tag* pTag,
        const QString& text,
        const QString& description,
        bool isNumericOrURL = false) {
    TagLib::ID3v2::UserTextIdentificationFrame* pFrame =
            findFirstUserTextIdentificationFrame(*pTag, description);
    if (nullptr != pFrame) {
        // Modify existing frame
        if (text.isEmpty()) {
            // Purge empty frames
            pTag->removeFrame(pFrame);
        } else {
            pFrame->setDescription(toTagLibString(description));
            pFrame->setText(toTagLibString(text));
        }
    } else {
        // Add a new (non-empty) frame
        if (!text.isEmpty()) {
            const TagLib::String::Type stringType =
                    getID3v2StringType(*pTag, isNumericOrURL);
            auto pFrame =
                    std::make_unique<TagLib::ID3v2::UserTextIdentificationFrame>(stringType);
            pFrame->setDescription(toTagLibString(description));
            pFrame->setText(toTagLibString(text));
            pTag->addFrame(pFrame.get());
            // Now that the plain pointer in pFrame is owned and managed by
            // pTag we need to release the ownership to avoid double deletion!
            pFrame.release();
        }
    }
}

// Bitmask of optional tag fields that should NOT be written into the
// common part of the tag through TagLib::Tag. For future extension
// it is safer to explicitly specify these exceptions!
// Usage: The write functions for ID3v2, MP4, APE and XiphComment tags
// have specialized code for some or all of the corresponding tag fields
// and it is not needed or even dangerous to use the common setters of
// TagLib::Tag.
enum WriteTagMask {
    WRITE_TAG_OMIT_NONE         = 0x00,
    WRITE_TAG_OMIT_TRACK_NUMBER = 0x01,
    WRITE_TAG_OMIT_YEAR         = 0x02,
    WRITE_TAG_OMIT_COMMENT      = 0x04,
};

void writeTrackMetadataIntoTag(
        TagLib::Tag* pTag,
        const TrackMetadata& trackMetadata,
        int writeMask) {
    DEBUG_ASSERT(pTag); // already validated before

    pTag->setArtist(toTagLibString(trackMetadata.getArtist()));
    pTag->setTitle(toTagLibString(trackMetadata.getTitle()));
    pTag->setAlbum(toTagLibString(trackMetadata.getAlbum()));
    pTag->setGenre(toTagLibString(trackMetadata.getGenre()));

    // Using setComment() from TagLib::Tag might have undesirable
    // effects if the tag type supports multiple comment fields for
    // different purposes, e.g. ID3v2. In this case setting the
    // comment here should be omitted.
    if (0 == (writeMask & WRITE_TAG_OMIT_COMMENT)) {
        pTag->setComment(toTagLibString(trackMetadata.getComment()));
    }

    // Specialized write functions for tags derived from Taglib::Tag might
    // be able to write the complete string from trackMetadata.getYear()
    // into the corresponding field. In this case parsing the year string
    // here should be omitted.
    if (0 == (writeMask & WRITE_TAG_OMIT_YEAR)) {
        // Set the numeric year if available
        const QDate yearDate(
                TrackMetadata::parseDateTime(trackMetadata.getYear()).date());
        if (yearDate.isValid()) {
            pTag->setYear(yearDate.year());
        }
    }

    // The numeric track number in TagLib::Tag does not reflect the total
    // number of tracks! Specialized write functions for tags derived from
    // Taglib::Tag might be able to handle both trackMetadata.getTrackNumber()
    // and trackMetadata.getTrackTotal(). In this case parsing the track
    // number string here is useless and should be omitted.
    if (0 == (writeMask & WRITE_TAG_OMIT_TRACK_NUMBER)) {
        // Set the numeric track number if available
        TrackNumbers parsedTrackNumbers;
        const TrackNumbers::ParseResult parseResult =
                TrackNumbers::parseFromString(trackMetadata.getTrackNumber(), &parsedTrackNumbers);
        if (TrackNumbers::ParseResult::VALID == parseResult) {
            pTag->setTrack(parsedTrackNumbers.getActual());
        }
    }
}

bool readMP4Atom(
        const TagLib::MP4::Tag& tag,
        const TagLib::String& key,
        QString* pValue = nullptr) {
    const TagLib::MP4::ItemListMap::ConstIterator it(
            getItemListMap(tag).find(key));
    if (it != getItemListMap(tag).end()) {
        if (nullptr != pValue) {
            *pValue = toQStringFirstNotEmpty((*it).second);
        }
        return true;
    } else {
        return false;
    }
}

// Unconditionally write the atom
void writeMP4Atom(
        TagLib::MP4::Tag* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (value.isEmpty()) {
        // Purge empty atoms
        pTag->itemListMap().erase(key);
    } else {
        TagLib::StringList strList(value);
        pTag->itemListMap()[key] = std::move(strList);
    }
}

// Conditionally write the atom if it already exists
void updateMP4Atom(
        TagLib::MP4::Tag* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (readMP4Atom(*pTag, key)) {
        writeMP4Atom(pTag, key, value);
    }
}

bool readAPEItem(
        const TagLib::APE::Tag& tag,
        const TagLib::String& key,
        QString* pValue = nullptr) {
    const TagLib::APE::ItemListMap::ConstIterator it(
            tag.itemListMap().find(key));
    if (it != tag.itemListMap().end() && !(*it).second.values().isEmpty()) {
        if (nullptr != pValue) {
            *pValue = toQStringFirstNotEmpty((*it).second.values());
        }
        return true;
    } else {
        return false;
    }
}

void writeAPEItem(
        TagLib::APE::Tag* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (value.isEmpty()) {
        // Purge empty items
        pTag->removeItem(key);
    } else {
        const bool replace = true;
        pTag->addValue(key, value, replace);
    }
}

bool readXiphCommentField(
        const TagLib::Ogg::XiphComment& tag,
        const TagLib::String& key,
        QString* pValue = nullptr) {
    const TagLib::Ogg::FieldListMap::ConstIterator it(
            tag.fieldListMap().find(key));
    if (it != tag.fieldListMap().end() && !(*it).second.isEmpty()) {
        if (nullptr != pValue) {
            *pValue = toQStringFirstNotEmpty((*it).second);
        }
        return true;
    } else {
        return false;
    }
}

// Unconditionally write the field
void writeXiphCommentField(
        TagLib::Ogg::XiphComment* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (value.isEmpty()) {
        // Purge empty fields
        pTag->removeField(key);
    } else {
        const bool replace = true;
        pTag->addField(key, value, replace);
    }
}

// Conditionally write the field if it already exists
void updateXiphCommentField(
        TagLib::Ogg::XiphComment* pTag,
        const TagLib::String& key,
        const TagLib::String& value) {
    if (readXiphCommentField(*pTag, key)) {
        writeXiphCommentField(pTag, key, value);
    }
}

} // anonymous namespace

void readTrackMetadataFromID3v2Tag(TrackMetadata* pTrackMetadata,
        const TagLib::ID3v2::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    readTrackMetadataFromTag(pTrackMetadata, tag);

    TagLib::ID3v2::CommentsFrame* pCommentsFrame =
            findFirstCommentsFrame(tag);
    if (nullptr != pCommentsFrame) {
        pTrackMetadata->setComment(toQString(*pCommentsFrame));
    } else {
        // Compatibility workaround: ffmpeg 3.1.x maps DESCRIPTION fields of
        // FLAC files with Vorbis Tags into TXXX frames labeled "comment"
        // upon conversion to MP3. This might also happen when transcoding
        // other file types to MP3 if ffmpeg is writing comments into this
        // non-standard ID3v2 text frame.
        // Note: The description string that identifies certain text frames
        // is case-insensitive. We do the lookup with an upper-case string
        // like for all other frames.
        TagLib::ID3v2::UserTextIdentificationFrame* pCommentFrame =
                findFirstUserTextIdentificationFrame(tag, "COMMENT");
        if (pCommentFrame != nullptr) {
            // The value is stored in the 2nd field
            pTrackMetadata->setComment(
                    toQString(pCommentFrame->fieldList()[1]));
        }
    }

    const TagLib::ID3v2::FrameList albumArtistFrame(tag.frameListMap()["TPE2"]);
    if (!albumArtistFrame.isEmpty()) {
        pTrackMetadata->setAlbumArtist(toQStringFirstNotEmpty(albumArtistFrame));
    }

    if (pTrackMetadata->getAlbum().isEmpty()) {
        const TagLib::ID3v2::FrameList originalAlbumFrame(
                tag.frameListMap()["TOAL"]);
        pTrackMetadata->setAlbum(toQStringFirstNotEmpty(originalAlbumFrame));
    }

    const TagLib::ID3v2::FrameList composerFrame(tag.frameListMap()["TCOM"]);
    if (!composerFrame.isEmpty()) {
        pTrackMetadata->setComposer(toQStringFirstNotEmpty(composerFrame));
    }

    const TagLib::ID3v2::FrameList groupingFrame(tag.frameListMap()["TIT1"]);
    if (!groupingFrame.isEmpty()) {
        pTrackMetadata->setGrouping(toQStringFirstNotEmpty(groupingFrame));
    }

    // ID3v2.4.0: TDRC replaces TYER + TDAT
    const QString recordingTime(
            toQStringFirstNotEmpty(tag.frameListMap()["TDRC"]));
    if ((4 <= tag.header()->majorVersion()) && !recordingTime.isEmpty()) {
            pTrackMetadata->setYear(recordingTime);
    } else {
        // Fallback to TYER + TDAT
        const QString recordingYear(
                toQStringFirstNotEmpty(tag.frameListMap()["TYER"]).trimmed());
        QString year(recordingYear);
        if (ID3V2_TYER_FORMAT.length() == recordingYear.length()) {
            const QString recordingDate(
                    toQStringFirstNotEmpty(tag.frameListMap()["TDAT"]).trimmed());
            if (ID3V2_TDAT_FORMAT.length() == recordingDate.length()) {
                const QDate date(
                        QDate::fromString(
                                recordingYear + recordingDate,
                                ID3V2_TYER_FORMAT + ID3V2_TDAT_FORMAT));
                if (date.isValid()) {
                    year = TrackMetadata::formatDate(date);
                }
            }
        }
        if (!year.isEmpty()) {
            pTrackMetadata->setYear(year);
        }
    }

    const TagLib::ID3v2::FrameList trackNumberFrame(tag.frameListMap()["TRCK"]);
    if (!trackNumberFrame.isEmpty()) {
        QString trackNumber;
        QString trackTotal;
        TrackNumbers::splitString(
                toQStringFirstNotEmpty(trackNumberFrame),
                &trackNumber,
                &trackTotal);
        pTrackMetadata->setTrackNumber(trackNumber);
        pTrackMetadata->setTrackTotal(trackTotal);
    }

    const TagLib::ID3v2::FrameList bpmFrame(tag.frameListMap()["TBPM"]);
    if (!bpmFrame.isEmpty()) {
        parseBpm(pTrackMetadata, toQStringFirstNotEmpty(bpmFrame));
        double bpmValue = pTrackMetadata->getBpm().getValue();
        // Some software use (or used) to write decimated values without comma,
        // so the number reads as 1352 or 14525 when it is 135.2 or 145.25
        double bpmValueOriginal = bpmValue;
        while (bpmValue > Bpm::kValueMax) {
            bpmValue /= 10.0;
        }
        if (bpmValue != bpmValueOriginal) {
            qWarning() << " Changing BPM on " << pTrackMetadata->getArtist() << " - " <<
                pTrackMetadata->getTitle() << " from " << bpmValueOriginal << " to " << bpmValue;
        }
        pTrackMetadata->setBpm(Bpm(bpmValue));
    }

    const TagLib::ID3v2::FrameList keyFrame(tag.frameListMap()["TKEY"]);
    if (!keyFrame.isEmpty()) {
        pTrackMetadata->setKey(toQStringFirstNotEmpty(keyFrame));
    }

    // Only read track gain (not album gain)
    TagLib::ID3v2::UserTextIdentificationFrame* pTrackGainFrame =
            findFirstUserTextIdentificationFrame(tag, "REPLAYGAIN_TRACK_GAIN");
    if (pTrackGainFrame && (2 <= pTrackGainFrame->fieldList().size())) {
        // The value is stored in the 2nd field
        parseTrackGain(pTrackMetadata,
                toQString(pTrackGainFrame->fieldList()[1]));
    }
    TagLib::ID3v2::UserTextIdentificationFrame* pTrackPeakFrame =
            findFirstUserTextIdentificationFrame(tag, "REPLAYGAIN_TRACK_PEAK");
    if (pTrackPeakFrame && (2 <= pTrackPeakFrame->fieldList().size())) {
        // The value is stored in the 2nd field
        parseTrackPeak(pTrackMetadata,
                toQString(pTrackPeakFrame->fieldList()[1]));
    }
}

void readTrackMetadataFromAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    readTrackMetadataFromTag(pTrackMetadata, tag);

    QString albumArtist;
    if (readAPEItem(tag, "Album Artist", &albumArtist)) {
        pTrackMetadata->setAlbumArtist(albumArtist);
    }

    QString composer;
    if (readAPEItem(tag, "Composer", &composer)) {
        pTrackMetadata->setComposer(composer);
    }

    QString grouping;
    if (readAPEItem(tag, "Grouping", &grouping)) {
        pTrackMetadata->setGrouping(grouping);
    }

    // The release date (ISO 8601 without 'T' separator between date and time)
    // according to the mapping used by MusicBrainz Picard.
    // http://wiki.hydrogenaud.io/index.php?title=APE_date
    // https://picard.musicbrainz.org/docs/mappings
    QString year;
    if (readAPEItem(tag, "Year", &year)) {
        pTrackMetadata->setYear(year);
    }

    QString trackNumber;
    if (readAPEItem(tag, "Track", &trackNumber)) {
        QString trackTotal;
        TrackNumbers::splitString(
                trackNumber,
                &trackNumber,
                &trackTotal);
        pTrackMetadata->setTrackNumber(trackNumber);
        pTrackMetadata->setTrackTotal(trackTotal);
    }

    QString bpm;
    if (readAPEItem(tag, "BPM", &bpm)) {
        parseBpm(pTrackMetadata, bpm);
    }

    // Only read track gain (not album gain)
    QString trackGain;
    if (readAPEItem(tag, "REPLAYGAIN_TRACK_GAIN", &trackGain)) {
        parseTrackGain(pTrackMetadata, trackGain);
    }
    QString trackPeak;
    if (readAPEItem(tag, "REPLAYGAIN_TRACK_PEAK", &trackPeak)) {
        parseTrackPeak(pTrackMetadata, trackPeak);
    }
}

void readTrackMetadataFromVorbisCommentTag(TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    readTrackMetadataFromTag(pTrackMetadata, tag);

    // Some applications (like puddletag up to version 1.0.5) write
    // "COMMENT" instead "DESCRIPTION".
    // Reference: http://www.xiph.org/vorbis/doc/v-comment.html
    if (!readXiphCommentField(tag, "DESCRIPTION")) { // recommended field (already read by TagLib)
        QString comment;
        if (readXiphCommentField(tag, "COMMENT", &comment)) { // alternative field
            pTrackMetadata->setComment(comment);
        }
    }

    QString albumArtist;
    if (readXiphCommentField(tag, "ALBUMARTIST", &albumArtist) || // recommended field
            readXiphCommentField(tag, "ALBUM_ARTIST", &albumArtist) || // alternative field (with underscore character)
            readXiphCommentField(tag, "ALBUM ARTIST", &albumArtist) || // alternative field (with space character)
            readXiphCommentField(tag, "ENSEMBLE", &albumArtist)) { // alternative field
        pTrackMetadata->setAlbumArtist(albumArtist);
    }

    QString composer;
    if (readXiphCommentField(tag, "COMPOSER", &composer)) {
        pTrackMetadata->setComposer(composer);
    }

    QString grouping;
    if (readXiphCommentField(tag, "GROUPING", &grouping)) {
        pTrackMetadata->setGrouping(grouping);
    }

    QString trackNumber;
    if (readXiphCommentField(tag, "TRACKNUMBER", &trackNumber)) {
        QString trackTotal;
        // Split the string, because some applications might decide
        // to store "<trackNumber>/<trackTotal>" in "TRACKNUMBER"
        // even if this is not recommended.
        TrackNumbers::splitString(
                trackNumber,
                &trackNumber,
                &trackTotal);
        if (!readXiphCommentField(tag, "TRACKTOTAL", &trackTotal)) { // recommended field
            (void)readXiphCommentField(tag, "TOTALTRACKS", &trackTotal); // alternative field
        }
        pTrackMetadata->setTrackNumber(trackNumber);
        pTrackMetadata->setTrackTotal(trackTotal);
    }

    // The release date formatted according to ISO 8601. Might
    // be followed by a space character and arbitrary text.
    // http://age.hobba.nl/audio/mirroredpages/ogg-tagging.html
    QString date;
    if (readXiphCommentField(tag, "DATE", &date)) {
        pTrackMetadata->setYear(date);
    }

    QString bpm;
    if (readXiphCommentField(tag, "TEMPO", &bpm) || // recommended field
            readXiphCommentField(tag, "BPM", &bpm)) { // alternative field
        parseBpm(pTrackMetadata, bpm);
    }

    // Only read track gain (not album gain)
    QString trackGain;
    if (readXiphCommentField(tag, "REPLAYGAIN_TRACK_GAIN", &trackGain)) {
        parseTrackGain(pTrackMetadata, trackGain);
    }
    QString trackPeak;
    if (readXiphCommentField(tag, "REPLAYGAIN_TRACK_PEAK", &trackPeak)) {
        parseTrackPeak(pTrackMetadata, trackPeak);
    }

    // Reading key code information
    // Unlike, ID3 tags, there's no standard or recommendation on how to store 'key' code
    //
    // Luckily, there are only a few tools for that, e.g., Rapid Evolution (RE).
    // Assuming no distinction between start and end key, RE uses a "INITIALKEY"
    // or a "KEY" vorbis comment.
    QString key;
    if (readXiphCommentField(tag, "INITIALKEY", &key) || // recommended field
            readXiphCommentField(tag, "KEY", &key)) { // alternative field
        pTrackMetadata->setKey(key);
    }
}

void readTrackMetadataFromMP4Tag(TrackMetadata* pTrackMetadata, const TagLib::MP4::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    readTrackMetadataFromTag(pTrackMetadata, tag);

    QString albumArtist;
    if (readMP4Atom(tag, "aART", &albumArtist)) {
        pTrackMetadata->setAlbumArtist(albumArtist);
    }

    QString composer;
    if (readMP4Atom(tag, "\251wrt", &composer)) {
        pTrackMetadata->setComposer(composer);
    }

    QString grouping;
    if (readMP4Atom(tag, "\251grp", &grouping)) {
        pTrackMetadata->setGrouping(grouping);
    }

    QString year;
    if (readMP4Atom(tag, "\251day", &year)) {
        pTrackMetadata->setYear(year);
    }

    // Read track number/total pair
    if (getItemListMap(tag).contains("trkn")) {
        const TagLib::MP4::Item::IntPair trknPair(getItemListMap(tag)["trkn"].toIntPair());
        const TrackNumbers trackNumbers(trknPair.first, trknPair.second);
        QString trackNumber;
        QString trackTotal;
        trackNumbers.toStrings(&trackNumber, &trackTotal);
        pTrackMetadata->setTrackNumber(trackNumber);
        pTrackMetadata->setTrackTotal(trackTotal);
    }

    QString bpm;
    if (readMP4Atom(tag, "----:com.apple.iTunes:BPM", &bpm)) {
        // This is the preferred field for storing the BPM
        // with fractional digits as a floating-point value.
        // If this field contains a valid value the integer
        // BPM value that might have been read before is
        // overwritten.
        parseBpm(pTrackMetadata, bpm);
    } else if (getItemListMap(tag).contains("tmpo")) {
            // Read the BPM as an integer value.
            const TagLib::MP4::Item& item = getItemListMap(tag)["tmpo"];
#if (TAGLIB_HAS_MP4_ATOM_TYPES)
            if (item.atomDataType() == TagLib::MP4::TypeInteger) {
                pTrackMetadata->setBpm(Bpm(item.toInt()));
            }
#else
            pTrackMetadata->setBpm(Bpm(item.toInt()));
#endif
    }

    // Only read track gain (not album gain)
    QString trackGain;
    if (readMP4Atom(tag, "----:com.apple.iTunes:replaygain_track_gain", &trackGain)) {
        parseTrackGain(pTrackMetadata, trackGain);
    }
    QString trackPeak;
    if (readMP4Atom(tag, "----:com.apple.iTunes:replaygain_track_peak", &trackPeak)) {
        parseTrackPeak(pTrackMetadata, trackPeak);
    }

    QString key;
    if (readMP4Atom(tag, "----:com.apple.iTunes:initialkey", &key) || // preferred (conforms to MixedInKey, Serato, Traktor)
            readMP4Atom(tag, "----:com.apple.iTunes:KEY", &key)) { // alternative (conforms to Rapid Evolution)
        pTrackMetadata->setKey(key);
    }
}

bool writeTrackMetadataIntoID3v2Tag(TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    const TagLib::ID3v2::Header* pHeader = pTag->header();
    if (!pHeader || (3 > pHeader->majorVersion())) {
        // only ID3v2.3.x and higher (currently only ID3v2.4.x) are supported
        return false;
    }

    // NOTE(uklotzde): Setting the comment for ID3v2 tags does
    // not work as expected when using TagLib 1.9.1 and must
    // be skipped! Otherwise special purpose comment fields
    // with a description like "iTunSMPB" might be overwritten.
    // Mixxx implements a special case handling for ID3v2 comment
    // frames (see below)
    writeTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_TRACK_NUMBER | WRITE_TAG_OMIT_YEAR | WRITE_TAG_OMIT_COMMENT);

    // Writing the common comments frame has been omitted (see above)
    writeID3v2CommentsFrame(pTag, trackMetadata.getComment());

    writeID3v2TextIdentificationFrame(pTag, "TRCK",
            TrackNumbers::joinStrings(
                    trackMetadata.getTrackNumber(),
                    trackMetadata.getTrackTotal()));

    // NOTE(uklotz): Need to overwrite the TDRC frame if it
    // already exists. TagLib (1.9.x) writes a TDRC frame
    // even for ID3v2.3.0 tags if the numeric year is set.
    if ((4 <= pHeader->majorVersion()) || !pTag->frameList("TDRC").isEmpty()) {
        writeID3v2TextIdentificationFrame(pTag, "TDRC",
                trackMetadata.getYear());
    }
    if (4 > pHeader->majorVersion()) {
        // Fallback to TYER + TDAT
        const QDate date(TrackMetadata::parseDate(trackMetadata.getYear()));
        if (date.isValid()) {
            // Valid date
            writeID3v2TextIdentificationFrame(pTag, "TYER", date.toString(ID3V2_TYER_FORMAT), true);
            writeID3v2TextIdentificationFrame(pTag, "TDAT", date.toString(ID3V2_TDAT_FORMAT), true);
        } else {
            // Fallback to calendar year
            bool calendarYearValid = false;
            const QString calendarYear(TrackMetadata::formatCalendarYear(trackMetadata.getYear(), &calendarYearValid));
            if (calendarYearValid) {
                writeID3v2TextIdentificationFrame(pTag, "TYER", calendarYear, true);
            }
        }
    }

    writeID3v2TextIdentificationFrame(pTag, "TPE2",
            trackMetadata.getAlbumArtist());
    writeID3v2TextIdentificationFrame(pTag, "TCOM",
            trackMetadata.getComposer());
    writeID3v2TextIdentificationFrame(pTag, "TIT1",
            trackMetadata.getGrouping());

    // According to the specification "The 'TBPM' frame contains the number
    // of beats per minute in the mainpart of the audio. The BPM is an
    // integer and represented as a numerical string."
    // Reference: http://id3.org/id3v2.3.0
    writeID3v2TextIdentificationFrame(pTag, "TBPM",
            formatBpmInteger(trackMetadata), true);

    writeID3v2TextIdentificationFrame(pTag, "TKEY", trackMetadata.getKey());

    // Only write track gain (not album gain)
    writeID3v2UserTextIdentificationFrame(
            pTag,
            formatTrackGain(trackMetadata),
            "REPLAYGAIN_TRACK_GAIN",
            true);
    writeID3v2UserTextIdentificationFrame(
            pTag,
            formatTrackPeak(trackMetadata),
            "REPLAYGAIN_TRACK_PEAK",
            true);

    return true;
}

bool writeTrackMetadataIntoAPETag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    writeTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_TRACK_NUMBER | WRITE_TAG_OMIT_YEAR);

    // NOTE(uklotzde): Overwrite the numeric track number in the common
    // part of the tag with the custom string from the track metadata
    // (pass-through without any further validation)
    writeAPEItem(pTag, "Track",
            toTagLibString(TrackNumbers::joinStrings(
                    trackMetadata.getTrackNumber(),
                    trackMetadata.getTrackTotal())));

    writeAPEItem(pTag, "Year",
            toTagLibString(trackMetadata.getYear()));

    writeAPEItem(pTag, "Album Artist",
            toTagLibString(trackMetadata.getAlbumArtist()));
    writeAPEItem(pTag, "Composer",
            toTagLibString(trackMetadata.getComposer()));
    writeAPEItem(pTag, "Grouping",
            toTagLibString(trackMetadata.getGrouping()));

    writeAPEItem(pTag, "BPM",
            toTagLibString(formatBpm(trackMetadata)));
    writeAPEItem(pTag, "REPLAYGAIN_TRACK_GAIN",
            toTagLibString(formatTrackGain(trackMetadata)));
    writeAPEItem(pTag, "REPLAYGAIN_TRACK_PEAK",
            toTagLibString(formatTrackPeak(trackMetadata)));

    return true;
}

bool writeTrackMetadataIntoXiphComment(TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    writeTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_TRACK_NUMBER | WRITE_TAG_OMIT_YEAR);

    // Write unambiguous fields
    writeXiphCommentField(pTag, "DATE",
            toTagLibString(trackMetadata.getYear()));
    writeXiphCommentField(pTag, "COMPOSER",
            toTagLibString(trackMetadata.getComposer()));
    writeXiphCommentField(pTag, "GROUPING",
            toTagLibString(trackMetadata.getGrouping()));
    writeXiphCommentField(pTag, "TRACKNUMBER",
            toTagLibString(trackMetadata.getTrackNumber()));
    writeXiphCommentField(pTag, "REPLAYGAIN_TRACK_GAIN",
            toTagLibString(formatTrackGain(trackMetadata)));
    writeXiphCommentField(pTag, "REPLAYGAIN_TRACK_PEAK",
            toTagLibString(formatTrackPeak(trackMetadata)));

    // According to https://wiki.xiph.org/Field_names "TRACKTOTAL" is
    // the proposed field name, but some applications use "TOTALTRACKS".
    const TagLib::String trackTotal(
            toTagLibString(trackMetadata.getTrackTotal()));
    writeXiphCommentField(pTag, "TRACKTOTAL", trackTotal); // recommended field
    updateXiphCommentField(pTag, "TOTALTRACKS", trackTotal); // alternative field

    const TagLib::String albumArtist(
            toTagLibString(trackMetadata.getAlbumArtist()));
    writeXiphCommentField(pTag, "ALBUMARTIST", albumArtist); // recommended field
    updateXiphCommentField(pTag, "ALBUM_ARTIST", albumArtist); // alternative field
    updateXiphCommentField(pTag, "ALBUM ARTIST", albumArtist); // alternative field
    updateXiphCommentField(pTag, "ENSEMBLE", albumArtist); // alternative field

    const TagLib::String bpm(
            toTagLibString(formatBpm(trackMetadata)));
    writeXiphCommentField(pTag, "TEMPO", bpm); // recommended field
    updateXiphCommentField(pTag, "BPM", bpm); // alternative field

    // Write both INITIALKEY and KEY
    const TagLib::String key(
            toTagLibString(trackMetadata.getKey()));
    writeXiphCommentField(pTag, "INITIALKEY", key); // recommended field
    updateXiphCommentField(pTag, "KEY", key); // alternative field

    return true;
}

bool writeTrackMetadataIntoMP4Tag(TagLib::MP4::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    writeTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_TRACK_NUMBER | WRITE_TAG_OMIT_YEAR);

    // Write track number/total pair
    QString trackNumberText;
    QString trackTotalText;
    TrackNumbers parsedTrackNumbers;
    const TrackNumbers::ParseResult parseResult =
            TrackNumbers::parseFromStrings(
                    trackMetadata.getTrackNumber(),
                    trackMetadata.getTrackTotal(),
                    &parsedTrackNumbers);
    switch (parseResult) {
    case TrackNumbers::ParseResult::EMPTY:
        pTag->itemListMap().erase("trkn");
        break;
    case TrackNumbers::ParseResult::VALID:
        pTag->itemListMap()["trkn"] = TagLib::MP4::Item(
                parsedTrackNumbers.getActual(),
                parsedTrackNumbers.getTotal());
        break;
    default:
        qWarning() << "Invalid track numbers:"
            << TrackNumbers::joinStrings(trackMetadata.getTrackNumber(), trackMetadata.getTrackTotal());
    }

    writeMP4Atom(pTag, "\251day", toTagLibString(trackMetadata.getYear()));

    writeMP4Atom(pTag, "aART", toTagLibString(trackMetadata.getAlbumArtist()));
    writeMP4Atom(pTag, "\251wrt", toTagLibString(trackMetadata.getComposer()));
    writeMP4Atom(pTag, "\251grp", toTagLibString(trackMetadata.getGrouping()));

    // Write both BPM fields (just in case)
    if (trackMetadata.getBpm().hasValue()) {
        // 16-bit integer value
        const int tmpoValue =
                mixxx::Bpm::valueToInteger(trackMetadata.getBpm().getValue());
        pTag->itemListMap()["tmpo"] = tmpoValue;
    } else {
        pTag->itemListMap().erase("tmpo");
    }
    writeMP4Atom(pTag, "----:com.apple.iTunes:BPM",
            toTagLibString(formatBpm(trackMetadata)));

    writeMP4Atom(pTag, "----:com.apple.iTunes:replaygain_track_gain",
            toTagLibString(formatTrackGain(trackMetadata)));
    writeMP4Atom(pTag, "----:com.apple.iTunes:replaygain_track_peak",
            toTagLibString(formatTrackPeak(trackMetadata)));

    const TagLib::String key(toTagLibString(trackMetadata.getKey()));
    writeMP4Atom(pTag, "----:com.apple.iTunes:initialkey", key); // preferred
    updateMP4Atom(pTag, "----:com.apple.iTunes:KEY", key); // alternative

    return true;
}

bool writeTrackMetadataIntoRIFFTag(TagLib::RIFF::Info::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    writeTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_NONE);

    return true;
}

namespace {

// Workaround for missing functionality in TagLib 1.11.x that
// doesn't support to read text chunks from AIFF files.
// See also:
// http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/AIFF/AIFF.html
// http://paulbourke.net/dataformats/audio/
//
//
class AiffFile: public TagLib::RIFF::AIFF::File {
  public:
    explicit AiffFile(TagLib::FileName fileName)
        : TagLib::RIFF::AIFF::File(fileName) {
    }

    void readTrackMetadataFromTextChunks(TrackMetadata* pTrackMetadata) /*non-const*/ {
        if (pTrackMetadata == nullptr) {
            return; // nothing to do
        }
        for(unsigned int i = 0; i < chunkCount(); ++i) {
            const TagLib::ByteVector chunkId(TagLib::RIFF::AIFF::File::chunkName(i));
            if (chunkId == "NAME") {
                pTrackMetadata->setTitle(decodeChunkText(
                        TagLib::RIFF::AIFF::File::chunkData(i)));
            } else if (chunkId == "AUTH") {
                pTrackMetadata->setArtist(decodeChunkText(
                        TagLib::RIFF::AIFF::File::chunkData(i)));
            } else if (chunkId == "ANNO") {
                pTrackMetadata->setComment(decodeChunkText(
                        TagLib::RIFF::AIFF::File::chunkData(i)));
            }
        }
    }

  private:
    // From the specs: 13. TEXT CHUNKS - NAME, AUTHOR, COPYRIGHT, ANNOTATION
    // "text: contains pure ASCII characters"
    // NOTE(uklotzde): In order to be independent of the currently defined
    // codec we use QString::fromLatin1() instead of QString::fromAscii()
    static QString decodeChunkText(const TagLib::ByteVector& chunkData) {
        return QString::fromLatin1(chunkData.data(), chunkData.size());
    }
};

} // anonymous namespace

Result readTrackMetadataAndCoverArtFromFile(TrackMetadata* pTrackMetadata, QImage* pCoverArt, QString fileName, FileType fileType) {
    if (fileType == FileType::UNKNOWN) {
        fileType = getFileTypeFromFileName(fileName);
    }

    qDebug() << "Reading tags from file" << fileName << "of type" << fileType
            << ":" << (pTrackMetadata ? "parsing" : "ignoring") << "track metadata"
            << "," << (pCoverArt ? "parsing" : "ignoring") << "cover art";

    // Rationale: If a file contains different types of tags only
    // a single type of tag will be read. Tag types are read in a
    // fixed order. Both track metadata and cover art will be read
    // from the same tag types. Only the first available tag type
    // is read and data in subsequent tags is ignored.

    switch (fileType) {
    case FileType::MP3:
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(fileName));
        if (readAudioProperties(pTrackMetadata, file)) {
            const TagLib::ID3v2::Tag* pID3v2Tag =
                    hasID3v2Tag(file) ? file.ID3v2Tag() : nullptr;
            if (pID3v2Tag) {
                readTrackMetadataFromID3v2Tag(pTrackMetadata, *pID3v2Tag);
                readCoverArtFromID3v2Tag(pCoverArt, *pID3v2Tag);
                return OK;
            } else {
                const TagLib::APE::Tag* pAPETag =
                        hasAPETag(file) ? file.APETag() : nullptr;
                if (pAPETag) {
                    readTrackMetadataFromAPETag(pTrackMetadata, *pAPETag);
                    readCoverArtFromAPETag(pCoverArt, *pAPETag);
                    return OK;
                } else {
                    // fallback
                    const TagLib::Tag* pTag(file.tag());
                    if (pTag) {
                        readTrackMetadataFromTag(pTrackMetadata, *pTag);
                        return OK;
                    }
                }
            }
        }
        break;
    }
    case FileType::MP4:
    {
        TagLib::MP4::File file(TAGLIB_FILENAME_FROM_QSTRING(fileName));
        if (readAudioProperties(pTrackMetadata, file)) {
            const TagLib::MP4::Tag* pMP4Tag = file.tag();
            if (pMP4Tag) {
                readTrackMetadataFromMP4Tag(pTrackMetadata, *pMP4Tag);
                readCoverArtFromMP4Tag(pCoverArt, *pMP4Tag);
                return OK;
            } else {
                // fallback
                const TagLib::Tag* pTag(file.tag());
                if (pTag) {
                    readTrackMetadataFromTag(pTrackMetadata, *pTag);
                    return OK;
                }
            }
        }
        break;
    }
    case FileType::FLAC:
    {
        TagLib::FLAC::File file(TAGLIB_FILENAME_FROM_QSTRING(fileName));
        // Read cover art directly from the file first. Will be
        // overwritten with cover art contained in on of the tags.
        if (pCoverArt != nullptr) {
            *pCoverArt = loadCoverArtImageFromVorbisCommentPictureList(file.pictureList());
        }
        if (readAudioProperties(pTrackMetadata, file)) {
            // VorbisComment tag takes precedence over ID3v2 tag
            TagLib::Ogg::XiphComment* pXiphComment =
                    hasXiphComment(file) ? file.xiphComment() : nullptr;
            if (pXiphComment) {
                readTrackMetadataFromVorbisCommentTag(pTrackMetadata, *pXiphComment);
                readCoverArtFromVorbisCommentTag(pCoverArt, *pXiphComment);
                return OK;
            } else {
                const TagLib::ID3v2::Tag* pID3v2Tag =
                        hasID3v2Tag(file) ? file.ID3v2Tag() : nullptr;
                if (pID3v2Tag) {
                    readTrackMetadataFromID3v2Tag(pTrackMetadata, *pID3v2Tag);
                    readCoverArtFromID3v2Tag(pCoverArt, *pID3v2Tag);
                    return OK;
                } else {
                    // fallback
                    const TagLib::Tag* pTag(file.tag());
                    if (pTag) {
                        readTrackMetadataFromTag(pTrackMetadata, *pTag);
                        return OK;
                    }
                }
            }
        }
        break;
    }
    case FileType::OGG:
    {
        TagLib::Ogg::Vorbis::File file(TAGLIB_FILENAME_FROM_QSTRING(fileName));
        if (readAudioProperties(pTrackMetadata, file)) {
            TagLib::Ogg::XiphComment* pXiphComment = file.tag();
            if (pXiphComment) {
                readTrackMetadataFromVorbisCommentTag(pTrackMetadata, *pXiphComment);
                readCoverArtFromVorbisCommentTag(pCoverArt, *pXiphComment);
                return OK;
            } else {
                // fallback
                const TagLib::Tag* pTag(file.tag());
                if (pTag) {
                    readTrackMetadataFromTag(pTrackMetadata, *pTag);
                    return OK;
                }
            }
        }
        break;
    }
#if (TAGLIB_HAS_OPUSFILE)
    case FileType::OPUS:
    {
        TagLib::Ogg::Opus::File file(TAGLIB_FILENAME_FROM_QSTRING(fileName));
        if (readAudioProperties(pTrackMetadata, file)) {
            TagLib::Ogg::XiphComment* pXiphComment = file.tag();
            if (pXiphComment) {
                readTrackMetadataFromVorbisCommentTag(pTrackMetadata, *pXiphComment);
                readCoverArtFromVorbisCommentTag(pCoverArt, *pXiphComment);
                 return OK;
            } else {
                // fallback
                const TagLib::Tag* pTag(file.tag());
                if (pTag) {
                    readTrackMetadataFromTag(pTrackMetadata, *pTag);
                    return OK;
                }
            }
        }
        break;
    }
#endif // TAGLIB_HAS_OPUSFILE
    case FileType::WV:
    {
        TagLib::WavPack::File file(TAGLIB_FILENAME_FROM_QSTRING(fileName));
        if (readAudioProperties(pTrackMetadata, file)) {
            const TagLib::APE::Tag* pAPETag =
                    hasAPETag(file) ? file.APETag() : nullptr;
            if (pAPETag) {
                readTrackMetadataFromAPETag(pTrackMetadata, *pAPETag);
                readCoverArtFromAPETag(pCoverArt, *pAPETag);
                return OK;
            } else {
                // fallback
                const TagLib::Tag* pTag(file.tag());
                if (pTag) {
                    readTrackMetadataFromTag(pTrackMetadata, *pTag);
                    return OK;
                }
            }
        }
        break;
    }
    case FileType::WAV:
    {
        TagLib::RIFF::WAV::File file(TAGLIB_FILENAME_FROM_QSTRING(fileName));
        if (readAudioProperties(pTrackMetadata, file)) {
#if (TAGLIB_HAS_WAV_ID3V2TAG)
            const TagLib::ID3v2::Tag* pID3v2Tag =
                    file.hasID3v2Tag() ? file.ID3v2Tag() : nullptr;
#else
            const TagLib::ID3v2::Tag* pID3v2Tag = file.tag();
#endif
            if (pID3v2Tag) {
                readTrackMetadataFromID3v2Tag(pTrackMetadata, *pID3v2Tag);
                readCoverArtFromID3v2Tag(pCoverArt, *pID3v2Tag);
                return OK;
            } else {
                // fallback
                const TagLib::RIFF::Info::Tag* pTag = file.InfoTag();
                if (pTag) {
                    readTrackMetadataFromRIFFTag(pTrackMetadata, *pTag);
                    return OK;
                }
            }
        }
        break;
    }
    case FileType::AIFF:
    {
        AiffFile file(TAGLIB_FILENAME_FROM_QSTRING(fileName));
        if (readAudioProperties(pTrackMetadata, file)) {
#if (TAGLIB_HAS_AIFF_HAS_ID3V2TAG)
            const TagLib::ID3v2::Tag* pID3v2Tag = file.hasID3v2Tag() ? file.tag() : nullptr;
#else
            const TagLib::ID3v2::Tag* pID3v2Tag = file.tag();
#endif
            if (pID3v2Tag) {
                readTrackMetadataFromID3v2Tag(pTrackMetadata, *pID3v2Tag);
                readCoverArtFromID3v2Tag(pCoverArt, *pID3v2Tag);
            } else {
                // fallback
                file.readTrackMetadataFromTextChunks(pTrackMetadata);
            }
            return OK;
        }
        break;
    }
    default:
        qWarning()
            << "Cannot read metadata from file"
            << fileName
            << "with unknown or unsupported file type"
            << fileType;
        return ERR;
    }

    qWarning() << "Failed to read track metadata from file" << fileName;
    return ERR;
}

// Encapsulates subtle differences between TagLib::File::save()
// and variants of this function in derived subclasses.
class TagSaver {
public:
    virtual ~TagSaver() {}

    virtual bool hasModifiedTags() const = 0;

    virtual bool saveModifiedTags() = 0;
};

class MpegTagSaver: public TagSaver {
public:
    MpegTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTagsBitmask(writeTrackMetadata(&m_file, trackMetadata)) {
    }
    ~MpegTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTagsBitmask != TagLib::MPEG::File::NoTags;
    }

    bool saveModifiedTags() override {
        // NOTE(uklotzde, 2016-08-28): Only save the tags that have
        // actually been modified! Otherwise TagLib 1.11 adds unwanted
        // ID3v1 tags, even if the file does not already contain those
        // legacy tags.
        return m_file.save(m_modifiedTagsBitmask);
    }

private:
    static int writeTrackMetadata(TagLib::MPEG::File* pFile, const TrackMetadata& trackMetadata) {
        int modifiedTagsBitmask = TagLib::MPEG::File::NoTags;
        if (pFile->isOpen()) {
            TagLib::ID3v2::Tag* pID3v2Tag = nullptr;
            if (hasAPETag(*pFile)) {
                if (writeTrackMetadataIntoAPETag(pFile->APETag(), trackMetadata)) {
                    modifiedTagsBitmask |= TagLib::MPEG::File::APE;
                }
                // Only write ID3v2 tag if it already exists
                pID3v2Tag = pFile->ID3v2Tag(false);
            } else {
                // Get or create  ID3v2 tag
                pID3v2Tag = pFile->ID3v2Tag(true);
            }
            if (writeTrackMetadataIntoID3v2Tag(pID3v2Tag, trackMetadata)) {
                modifiedTagsBitmask |= TagLib::MPEG::File::ID3v2;
            }
        }
        return modifiedTagsBitmask;
    }

    TagLib::MPEG::File m_file;
    int m_modifiedTagsBitmask;
};

class Mp4TagSaver: public TagSaver {
public:
    Mp4TagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(writeTrackMetadata(&m_file, trackMetadata)) {
    }
    ~Mp4TagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool writeTrackMetadata(TagLib::MP4::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && writeTrackMetadataIntoMP4Tag(pFile->tag(), trackMetadata);
    }

    TagLib::MP4::File m_file;
    bool m_modifiedTags;
};

class FlacTagSaver: public TagSaver {
public:
    FlacTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(writeTrackMetadata(&m_file, trackMetadata)) {
    }
    ~FlacTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool writeTrackMetadata(TagLib::FLAC::File* pFile, const TrackMetadata& trackMetadata) {
        bool modifiedTags = false;
        if (pFile->isOpen()) {
            TagLib::Ogg::XiphComment* pXiphComment = nullptr;
            if (hasID3v2Tag(*pFile)) {
                modifiedTags |= writeTrackMetadataIntoID3v2Tag(pFile->ID3v2Tag(), trackMetadata);
                // Only write VorbisComment tag if it already exists
                pXiphComment = pFile->xiphComment(false);
            } else {
                // Get or create VorbisComment tag
                pXiphComment = pFile->xiphComment(true);
            }
            modifiedTags |= writeTrackMetadataIntoXiphComment(pXiphComment, trackMetadata);
        }
        return modifiedTags;
    }

    TagLib::FLAC::File m_file;
    bool m_modifiedTags;
};

class OggTagSaver: public TagSaver {
public:
    OggTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(writeTrackMetadata(&m_file, trackMetadata)) {
    }
    ~OggTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool writeTrackMetadata(TagLib::Ogg::Vorbis::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && writeTrackMetadataIntoXiphComment(pFile->tag(), trackMetadata);
    }

    TagLib::Ogg::Vorbis::File m_file;
    bool m_modifiedTags;
};

#if (TAGLIB_HAS_OPUSFILE)
class OpusTagSaver: public TagSaver {
public:
    OpusTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(writeTrackMetadata(&m_file, trackMetadata)) {
    }
    ~OpusTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool writeTrackMetadata(TagLib::Ogg::Opus::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && writeTrackMetadataIntoXiphComment(pFile->tag(), trackMetadata);
    }

    TagLib::Ogg::Opus::File m_file;
    bool m_modifiedTags;
};
#endif // TAGLIB_HAS_OPUSFILE

class WavPackTagSaver: public TagSaver {
public:
    WavPackTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(writeTrackMetadata(&m_file, trackMetadata)) {
    }
    ~WavPackTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool writeTrackMetadata(TagLib::WavPack::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && writeTrackMetadataIntoAPETag(pFile->APETag(true), trackMetadata);
    }

    TagLib::WavPack::File m_file;
    bool m_modifiedTags;
};

class WavTagSaver: public TagSaver {
public:
    WavTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(writeTrackMetadata(&m_file, trackMetadata)) {
    }
    ~WavTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool writeTrackMetadata(TagLib::RIFF::WAV::File* pFile, const TrackMetadata& trackMetadata) {
        bool modifiedTags = false;
        if (pFile->isOpen()) {
            // Write into all available tags
#if (TAGLIB_HAS_WAV_ID3V2TAG)
            modifiedTags |= writeTrackMetadataIntoID3v2Tag(pFile->ID3v2Tag(), trackMetadata);
#else
            modifiedTags |= writeTrackMetadataIntoID3v2Tag(pFile->tag(), trackMetadata);
#endif
            modifiedTags |= writeTrackMetadataIntoRIFFTag(pFile->InfoTag(), trackMetadata);
        }
        return modifiedTags;
    }

    TagLib::RIFF::WAV::File m_file;
    bool m_modifiedTags;
};

class AiffTagSaver: public TagSaver {
public:
    AiffTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(writeTrackMetadata(&m_file, trackMetadata)) {
    }
    ~AiffTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool writeTrackMetadata(TagLib::RIFF::AIFF::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && writeTrackMetadataIntoID3v2Tag(pFile->tag(), trackMetadata);
    }

    TagLib::RIFF::AIFF::File m_file;
    bool m_modifiedTags;
};

Result writeTrackMetadataIntoFile(const TrackMetadata& trackMetadata, QString fileName, FileType fileType) {
    if (fileType == FileType::UNKNOWN) {
        fileType = getFileTypeFromFileName(fileName);
    }

    qDebug() << "Writing track metadata into file"
            << fileName
            << "of type"
            << fileType;

    std::unique_ptr<TagSaver> pTagSaver;
    switch (fileType) {
    case FileType::MP3:
    {
        pTagSaver = std::make_unique<MpegTagSaver>(fileName, trackMetadata);
        break;
    }
    case FileType::MP4:
    {
        pTagSaver = std::make_unique<Mp4TagSaver>(fileName, trackMetadata);
        break;
    }
    case FileType::FLAC:
    {
        pTagSaver = std::make_unique<FlacTagSaver>(fileName, trackMetadata);
        break;
    }
    case FileType::OGG:
    {
        pTagSaver = std::make_unique<OggTagSaver>(fileName, trackMetadata);
        break;
    }
#if (TAGLIB_HAS_OPUSFILE)
    case FileType::OPUS:
    {
        pTagSaver = std::make_unique<OpusTagSaver>(fileName, trackMetadata);
        break;
    }
#endif // TAGLIB_HAS_OPUSFILE
    case FileType::WV:
    {
        pTagSaver = std::make_unique<WavPackTagSaver>(fileName, trackMetadata);
        break;
    }
    case FileType::WAV:
    {
        pTagSaver = std::make_unique<WavTagSaver>(fileName, trackMetadata);
        break;
    }
    case FileType::AIFF:
    {
        pTagSaver = std::make_unique<AiffTagSaver>(fileName, trackMetadata);
        break;
    }
    default:
        qWarning()
            << "Cannot write metadata into tags of file"
            << fileName
            << "with unknown or unsupported file type"
            << fileType;
        return ERR;
    }

    if (pTagSaver->hasModifiedTags()) {
        if (pTagSaver->saveModifiedTags()) {
            return OK;
        } else {
            qWarning() << "Failed to save tags of file" << fileName;
            return ERR;
        }
    } else {
        qWarning() << "Failed to modify tags of file" << fileName;
        return ERR;
    }
}

} // namespace taglib

} //namespace mixxx
