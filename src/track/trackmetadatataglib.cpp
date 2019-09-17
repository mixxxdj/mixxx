#include "track/trackmetadatataglib.h"

#include "track/tracknumbers.h"

#include "util/assert.h"
#include "util/duration.h"
#include "util/logger.h"
#include "util/memory.h"


// TagLib has support for has<TagType>() style functions since version 1.9
#define TAGLIB_HAS_TAG_CHECK \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))

// TagLib has support for length in milliseconds since version 1.10
#define TAGLIB_HAS_LENGTH_IN_MILLISECONDS \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 10))

// TagLib has support for XiphComment::pictureList() since version 1.11
#define TAGLIB_HAS_VORBIS_COMMENT_PICTURES \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 11))

#include <taglib/tfile.h>
#include <taglib/tmap.h>
#include <taglib/tstringlist.h>

#include <taglib/commentsframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacpicture.h>

#include <array>


namespace mixxx {

namespace {

Logger kLogger("TagLib");

// Only ID3v2.3 and ID3v2.4 are supported for both importing and
// exporting text frames. ID3v2.2 uses different frame identifiers,
// i.e. only 3 instead of 4 characters.
// https://en.wikipedia.org/wiki/ID3#ID3v2
// http://id3.org/Developer%20Information
const unsigned int kMinID3v2Version = 3;

bool checkID3v2HeaderVersionSupported(const TagLib::ID3v2::Header& header) {
    if (header.majorVersion() < kMinID3v2Version) {
        kLogger.warning().noquote()
                << QString("ID3v2.%1 is only partially supported - please convert your file tags to at least ID3v2.%2").arg(
                        QString::number(header.majorVersion()),
                        QString::number(kMinID3v2Version));
        return false;
    } else {
        return true;
    }
}

} // anonymous namespace

namespace taglib {

// Deduce the TagLib file type from the file name
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

QDebug operator<<(QDebug debug, FileType fileType) {
    return debug << static_cast<std::underlying_type<FileType>::type>(fileType);
}

bool hasID3v1Tag(TagLib::MPEG::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasID3v1Tag();
#else
    return file.ID3v1Tag() != nullptr;
#endif
}

bool hasID3v2Tag(TagLib::MPEG::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasID3v2Tag();
#else
    return file.ID3v2Tag() != nullptr;
#endif
}

bool hasAPETag(TagLib::MPEG::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasAPETag();
#else
    return file.APETag() != nullptr;
#endif
}

bool hasID3v2Tag(TagLib::FLAC::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasID3v2Tag();
#else
    return file.ID3v2Tag() != nullptr;
#endif
}

bool hasXiphComment(TagLib::FLAC::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasXiphComment();
#else
    return file.xiphComment() != nullptr;
#endif
}

bool hasAPETag(TagLib::WavPack::File& file) {
#if (TAGLIB_HAS_TAG_CHECK)
    return file.hasAPETag();
#else
    return file.APETag() != nullptr;
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

inline QString toQString(const TagLib::String& tString) {
    if (tString.isNull()) {
        // null -> null
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
        if (pFrame) {
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
    if (str.isNull()) {
        return TagLib::String::null;
    } else {
        return TagLib::String(qba.constData(), TagLib::String::UTF8);
    }
}

inline QString formatBpm(const TrackMetadata& trackMetadata) {
    return Bpm::valueToString(trackMetadata.getTrackInfo().getBpm().getValue());
}

inline
QString formatBpmInteger(const TrackMetadata& trackMetadata) {
    return QString::number(Bpm::valueToInteger(trackMetadata.getTrackInfo().getBpm().getValue()));
}

bool parseBpm(TrackMetadata* pTrackMetadata, QString sBpm) {
    DEBUG_ASSERT(pTrackMetadata);

    bool isBpmValid = false;
    const double bpmValue = Bpm::valueFromString(sBpm, &isBpmValid);
    if (isBpmValid) {
        pTrackMetadata->refTrackInfo().setBpm(Bpm(bpmValue));
    }
    return isBpmValid;
}

inline
QString formatReplayGainGain(const ReplayGain& replayGain) {
    return ReplayGain::ratioToString(replayGain.getRatio());
}

inline
QString formatTrackGain(const TrackMetadata& trackMetadata) {
    return formatReplayGainGain(trackMetadata.getTrackInfo().getReplayGain());
}

bool parseReplayGainGain(
        ReplayGain* pReplayGain,
        const QString& dbGain) {
    DEBUG_ASSERT(pReplayGain);

    bool isRatioValid = false;
    double ratio = ReplayGain::ratioFromString(dbGain, &isRatioValid);
    if (isRatioValid) {
        // Some applications (e.g. Rapid Evolution 3) write a replay gain
        // of 0 dB even if the replay gain is undefined. To be safe we
        // ignore this special value and instead prefer to recalculate
        // the replay gain.
        if (ratio == ReplayGain::kRatio0dB) {
            // special case
            kLogger.info() << "Ignoring possibly undefined gain:" << dbGain;
            ratio = ReplayGain::kRatioUndefined;
        }
        pReplayGain->setRatio(ratio);
    }
    return isRatioValid;
}

bool parseTrackGain(
        TrackMetadata* pTrackMetadata,
        const QString& dbGain) {
    DEBUG_ASSERT(pTrackMetadata);

    ReplayGain replayGain(pTrackMetadata->getTrackInfo().getReplayGain());
    bool isRatioValid = parseReplayGainGain(&replayGain, dbGain);
    if (isRatioValid) {
        pTrackMetadata->refTrackInfo().setReplayGain(replayGain);
    }
    return isRatioValid;
}

inline
QString formatReplayGainPeak(const ReplayGain& replayGain) {
    return ReplayGain::peakToString(replayGain.getPeak());
}

inline
bool hasTrackPeak(const TrackMetadata& trackMetadata) {
    return trackMetadata.getTrackInfo().getReplayGain().hasPeak();
}

inline
QString formatTrackPeak(const TrackMetadata& trackMetadata) {
    return formatReplayGainPeak(trackMetadata.getTrackInfo().getReplayGain());
}

bool parseReplayGainPeak(
        ReplayGain* pReplayGain,
        const QString& strPeak) {
    DEBUG_ASSERT(pReplayGain);

    bool isPeakValid = false;
    const CSAMPLE peak = ReplayGain::peakFromString(strPeak, &isPeakValid);
    if (isPeakValid) {
        pReplayGain->setPeak(peak);
    }
    return isPeakValid;
}

bool parseTrackPeak(
        TrackMetadata* pTrackMetadata,
        const QString& strPeak) {
    DEBUG_ASSERT(pTrackMetadata);

    ReplayGain replayGain(pTrackMetadata->getTrackInfo().getReplayGain());
    bool isPeakValid = parseReplayGainPeak(&replayGain, strPeak);
    if (isPeakValid) {
        pTrackMetadata->refTrackInfo().setReplayGain(replayGain);
    }
    return isPeakValid;
}

inline
bool hasAlbumGain(const TrackMetadata& trackMetadata) {
    return trackMetadata.getAlbumInfo().getReplayGain().hasRatio();
}

inline
QString formatAlbumGain(const TrackMetadata& trackMetadata) {
    return formatReplayGainGain(trackMetadata.getAlbumInfo().getReplayGain());
}

bool parseAlbumGain(
        TrackMetadata* pTrackMetadata,
        const QString& dbGain) {
    DEBUG_ASSERT(pTrackMetadata);

    ReplayGain replayGain(pTrackMetadata->getAlbumInfo().getReplayGain());
    bool isRatioValid = parseReplayGainGain(&replayGain, dbGain);
    if (isRatioValid) {
        pTrackMetadata->refAlbumInfo().setReplayGain(replayGain);
    }
    return isRatioValid;
}

inline
bool hasAlbumPeak(const TrackMetadata& trackMetadata) {
    return trackMetadata.getAlbumInfo().getReplayGain().hasPeak();
}

inline
QString formatAlbumPeak(const TrackMetadata& trackMetadata) {
    return formatReplayGainPeak(trackMetadata.getAlbumInfo().getReplayGain());
}

bool parseAlbumPeak(
        TrackMetadata* pTrackMetadata,
        const QString& strPeak) {
    DEBUG_ASSERT(pTrackMetadata);

    ReplayGain replayGain(pTrackMetadata->getAlbumInfo().getReplayGain());
    bool isPeakValid = parseReplayGainPeak(&replayGain, strPeak);
    if (isPeakValid) {
        pTrackMetadata->refAlbumInfo().setReplayGain(replayGain);
    }
    return isPeakValid;
}

void readAudioProperties(
        TrackMetadata* pTrackMetadata,
        const TagLib::AudioProperties& audioProperties) {
    DEBUG_ASSERT(pTrackMetadata);

    // NOTE(uklotzde): All audio properties will be updated
    // with the actual (and more precise) values when reading
    // the audio data for this track. Often those properties
    // stored in tags don't match with the corresponding
    // audio data in the file.
    pTrackMetadata->setChannels(AudioSignal::ChannelCount(audioProperties.channels()));
    pTrackMetadata->setSampleRate(AudioSignal::SampleRate(audioProperties.sampleRate()));
    pTrackMetadata->setBitrate(AudioSource::Bitrate(audioProperties.bitrate()));
#if (TAGLIB_HAS_LENGTH_IN_MILLISECONDS)
    const auto duration = Duration::fromMillis(audioProperties.lengthInMilliseconds());
#else
    const auto duration = Duration::fromSeconds(audioProperties.length());
#endif
    pTrackMetadata->setDuration(duration);
}

// Workaround for missing const member function in TagLib
inline const TagLib::MP4::ItemListMap& getItemListMap(const TagLib::MP4::Tag& tag) {
    return const_cast<TagLib::MP4::Tag&>(tag).itemListMap();
}

inline QImage loadImageFromByteVector(
        const TagLib::ByteVector& imageData,
        const char* format = nullptr) {
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

bool parseBase64EncodedVorbisCommentPicture(
        TagLib::FLAC::Picture* pPicture,
        const TagLib::String& base64Encoded) {
    DEBUG_ASSERT(pPicture);
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

TagLib::String::Type getID3v2StringType(const TagLib::ID3v2::Tag& tag, bool isNumericOrURL = false) {
    TagLib::String::Type stringType;
    // For an overview of the character encodings supported by
    // the different ID3v2 versions please refer to the following
    // resources:
    // http://en.wikipedia.org/wiki/ID3#ID3v2
    // http://id3.org/id3v2.3.0
    // http://id3.org/id3v2.4.0-structure
    if (tag.header()->majorVersion() >= 4) {
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
        const QString& description,
        bool preferNotEmpty = true) {
    TagLib::ID3v2::CommentsFrame* pFirstFrame = nullptr;
    // Bind the const-ref result to avoid a local copy
    const TagLib::ID3v2::FrameList& commentsFrames =
            tag.frameListMap()["COMM"];
    for (TagLib::ID3v2::FrameList::ConstIterator it(commentsFrames.begin());
            it != commentsFrames.end(); ++it) {
        auto pFrame =
                dynamic_cast<TagLib::ID3v2::CommentsFrame*>(*it);
        if (pFrame) {
            const QString frameDescription(
                    toQString(pFrame->description()));
            if (0 == frameDescription.compare(
                    description, Qt::CaseInsensitive)) {
                if (preferNotEmpty && pFrame->toString().isEmpty()) {
                    // we might need the first matching frame later
                    // even if it is empty
                    if (!pFirstFrame) {
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

TagLib::ID3v2::CommentsFrame* findFirstCommentsFrameWithoutDescription(
        const TagLib::ID3v2::Tag& tag) {
    return findFirstCommentsFrame(tag, QString());
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
        if (pFrame) {
            const QString frameDescription(
                    toQString(pFrame->description()));
            if (0 == frameDescription.compare(
                    description, Qt::CaseInsensitive)) {
                if (preferNotEmpty && pFrame->toString().isEmpty()) {
                    // we might need the first matching frame later
                    // even if it is empty
                    if (!pFirstFrame) {
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

inline
QString readFirstUserTextIdentificationFrame(
        const TagLib::ID3v2::Tag& tag,
        const QString& description) {
    const TagLib::ID3v2::UserTextIdentificationFrame* pTextFrame =
            findFirstUserTextIdentificationFrame(tag, description);
    if (pTextFrame && (pTextFrame->fieldList().size() > 1)) {
        // The actual value is stored in the 2nd field
        return toQString(pTextFrame->fieldList()[1]);
    } else {
        return QString();
    }
}

// Deletes all TXXX frame with the given description (case-insensitive).
int removeUserTextIdentificationFrames(
        TagLib::ID3v2::Tag* pTag,
        const QString& description) {
    DEBUG_ASSERT(pTag);
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
            if (pFrame) {
                const QString frameDescription(
                        toQString(pFrame->description()));
                if (0 == frameDescription.compare(
                        description, Qt::CaseInsensitive)) {
                    if (kLogger.debugEnabled()) {
                        kLogger.debug()
                                << "Removing ID3v2 TXXX frame:"
                                << toQString(pFrame->description());
                    }
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

bool writeID3v2TextIdentificationFrameStringIfNotNull(
        TagLib::ID3v2::Tag* pTag,
        const TagLib::ByteVector &id,
        const QString& text) {
    if (text.isNull()) {
        return false;
    } else {
        writeID3v2TextIdentificationFrame(pTag, id, text);
        return true;
    }
}

void writeID3v2CommentsFrame(
        TagLib::ID3v2::Tag* pTag,
        const QString& text,
        const QString& description,
        bool isNumericOrURL = false) {
    TagLib::ID3v2::CommentsFrame* pFrame =
            findFirstCommentsFrame(*pTag, description, true);
    if (pFrame) {
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
        kLogger.warning() << "Removed" << numberOfRemovedCommentFrames
                << "non-standard ID3v2 TXXX comment frames";
    }
}

void writeID3v2CommentsFrameWithoutDescription(
        TagLib::ID3v2::Tag* pTag,
        const QString& text,
        bool isNumericOrURL = false) {
    writeID3v2CommentsFrame(pTag, text, QString(), isNumericOrURL);
}

void writeID3v2UserTextIdentificationFrame(
        TagLib::ID3v2::Tag* pTag,
        const QString& description,
        const QString& text,
        bool isNumericOrURL = false) {
    TagLib::ID3v2::UserTextIdentificationFrame* pFrame =
            findFirstUserTextIdentificationFrame(*pTag, description);
    if (pFrame) {
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

bool readMP4Atom(
        const TagLib::MP4::Tag& tag,
        const TagLib::String& key,
        QString* pValue = nullptr) {
    const TagLib::MP4::ItemListMap::ConstIterator it(
            getItemListMap(tag).find(key));
    if (it != getItemListMap(tag).end()) {
        if (pValue) {
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
        if (pValue) {
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
        QString* pValue) {
    const TagLib::Ogg::FieldListMap::ConstIterator it(
            tag.fieldListMap().find(key));
    if (it != tag.fieldListMap().end() && !(*it).second.isEmpty()) {
        if (pValue) {
            *pValue = toQStringFirstNotEmpty((*it).second);
        }
        return true;
    } else {
        return false;
    }
}

inline
bool hasXiphCommentField(
        const TagLib::Ogg::XiphComment& tag,
        const TagLib::String& key) {
    return readXiphCommentField(tag, key, nullptr);
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
    if (hasXiphCommentField(*pTag, key)) {
        writeXiphCommentField(pTag, key, value);
    }
}

} // anonymous namespace

bool readAudioProperties(
        TrackMetadata* pTrackMetadata,
        const TagLib::File& file) {
    if (!file.isValid()) {
        kLogger.warning() << "Cannot read audio properties from inaccessible/unreadable/invalid file:" << file.name();
        return false;
    }
    if (!pTrackMetadata) {
        // implicitly successful
        return true;
    }
    const TagLib::AudioProperties* pAudioProperties =
            file.audioProperties();
    if (!pAudioProperties) {
        kLogger.warning() << "Failed to read audio properties from file"
                << file.name();
        return false;
    }
    readAudioProperties(pTrackMetadata, *pAudioProperties);
    return true;
}

QImage importCoverImageFromVorbisCommentPictureList(
        const TagLib::List<TagLib::FLAC::Picture*>& pictures) {
    if (pictures.isEmpty()) {
        if (kLogger.debugEnabled()) {
            kLogger.debug() << "VorbisComment picture list is empty";
        }
        return QImage();
    }

    for (const auto coverArtType: kPreferredVorbisCommentPictureTypes) {
        for (const auto pPicture: pictures) {
            DEBUG_ASSERT(pPicture); // trust TagLib
            if (pPicture->type() == coverArtType) {
                const QImage image(loadImageFromVorbisCommentPicture(*pPicture));
                if (image.isNull()) {
                    kLogger.warning() << "Failed to load image from VorbisComment picture of type" << pPicture->type();
                    continue;
                } else {
                    return image; // success
                }
            }
        }
    }

    // Fallback: No best match -> Create image from first loadable picture of any type
    for (const auto pPicture: pictures) {
        DEBUG_ASSERT(pPicture); // trust TagLib
        const QImage image(loadImageFromVorbisCommentPicture(*pPicture));
        if (image.isNull()) {
            kLogger.warning()
                    << "Failed to load image from VorbisComment picture of type"
                    << pPicture->type();
            continue;
        } else {
            return image; // success
        }
    }

    kLogger.warning()
            << "Failed to load cover art image from VorbisComment pictures";
    return QImage();
}

template<typename T>
const T* downcastID3v2Frame(TagLib::ID3v2::Frame* frame) {
    DEBUG_ASSERT(frame);
    // We need to use a safe dynamic_cast at runtime instead of an unsafe
    // static_cast at compile time to detect unexpected frame subtypes!
    // See also: https://bugs.launchpad.net/mixxx/+bug/1774790
    const T* downcastFrame = dynamic_cast<T*>(frame);
    VERIFY_OR_DEBUG_ASSERT(downcastFrame) {
        // This should only happen when reading corrupt or malformed files
        kLogger.warning()
                << "Unexpected ID3v2"
                << frame->frameID().data()
                << "frame type";
    }
    return downcastFrame;
}

void importCoverImageFromID3v2Tag(QImage* pCoverArt, const TagLib::ID3v2::Tag& tag) {
    if (!pCoverArt) {
        return; // nothing to do
    }

    const auto iterAPIC = tag.frameListMap().find("APIC");
    if ((iterAPIC == tag.frameListMap().end()) || iterAPIC->second.isEmpty()) {
        if (kLogger.debugEnabled()) {
            kLogger.debug() << "No cover art: None or empty list of ID3v2 APIC frames";
        }
        return; // abort
    }

    const TagLib::ID3v2::FrameList pFrames = iterAPIC->second;
    for (const auto coverArtType: kPreferredID3v2PictureTypes) {
        for (const auto pFrame: pFrames) {
            const auto* pApicFrame =
                    downcastID3v2Frame<TagLib::ID3v2::AttachedPictureFrame>(pFrame);
            if (pApicFrame && (pApicFrame->type() == coverArtType)) {
                QImage image(loadImageFromID3v2PictureFrame(*pApicFrame));
                if (image.isNull()) {
                    kLogger.warning()
                            << "Failed to load image from ID3v2 APIC frame of type"
                            << pApicFrame->type();
                    continue;
                } else {
                    *pCoverArt = image;
                    return; // success
                }
            }
        }
    }

    // Fallback: No best match -> Simply select the 1st loadable image
    for (const auto pFrame: pFrames) {
        const auto* pApicFrame =
                downcastID3v2Frame<TagLib::ID3v2::AttachedPictureFrame>(pFrame);
        if (pApicFrame) {
            const QImage image(loadImageFromID3v2PictureFrame(*pApicFrame));
            if (image.isNull()) {
                kLogger.warning()
                        << "Failed to load image from ID3v2 APIC frame of type"
                        << pApicFrame->type();
                continue;
            } else {
                *pCoverArt = image;
                return; // success
            }
        }
    }
}

void importCoverImageFromAPETag(QImage* pCoverArt, const TagLib::APE::Tag& tag) {
    if (!pCoverArt) {
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
                kLogger.warning()
                        << "Failed to load image from APE tag";
            } else {
                *pCoverArt = image; // success
            }
        }
    }
}

void importCoverImageFromVorbisCommentTag(QImage* pCoverArt, TagLib::Ogg::XiphComment& tag) {
    if (!pCoverArt) {
        return; // nothing to do
    }

#if (TAGLIB_HAS_VORBIS_COMMENT_PICTURES)
    const QImage image=
            importCoverImageFromVorbisCommentPictureList(tag.pictureList());
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
            kLogger.warning()
                    << "Taking legacy code path for reading cover art from VorbisComment field METADATA_BLOCK_PICTURE";
        }
#endif
        for (const auto& base64Encoded: base64EncodedList) {
            TagLib::FLAC::Picture picture;
            if (parseBase64EncodedVorbisCommentPicture(&picture, base64Encoded)) {
                const QImage image(loadImageFromVorbisCommentPicture(picture));
                if (image.isNull()) {
                    kLogger.warning()
                            << "Failed to load image from VorbisComment picture of type"
                            << picture.type();
                    continue;
                } else {
                    *pCoverArt = image;
                    return; // done
                }
            } else {
                kLogger.warning()
                        << "Failed to parse picture from VorbisComment metadata block";
                continue;
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
            kLogger.warning() << "Fallback: Trying to parse image from deprecated VorbisComment field COVERART";
        }
        for (const auto& base64Encoded: base64EncodedList) {
            const QImage image(parseBase64EncodedVorbisCommentImage(base64Encoded));
            if (image.isNull()) {
                kLogger.warning()
                        << "Failed to parse image from deprecated VorbisComment field COVERART";
                continue;
            } else {
                *pCoverArt = image;
                return; // done
            }
        }
    }
    if (kLogger.debugEnabled()) {
        kLogger.debug() << "No cover art found in VorbisComment tag";
    }
}

void importCoverImageFromMP4Tag(QImage* pCoverArt, const TagLib::MP4::Tag& tag) {
    if (!pCoverArt) {
        return; // nothing to do
    }

    if (getItemListMap(tag).contains("covr")) {
        TagLib::MP4::CoverArtList coverArtList =
                getItemListMap(tag)["covr"].toCoverArtList();
        for (const auto& coverArt: coverArtList) {
            const QImage image(loadImageFromByteVector(coverArt.data()));
            if (image.isNull()) {
                kLogger.warning()
                        << "Failed to load image from MP4 atom covr";
                continue;
            } else {
                *pCoverArt = image;
                return; // done
            }
        }
    }
}

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::Tag& tag,
        int readMask) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    pTrackMetadata->refTrackInfo().setTitle(toQString(tag.title()));
    pTrackMetadata->refTrackInfo().setArtist(toQString(tag.artist()));
    pTrackMetadata->refTrackInfo().setGenre(toQString(tag.genre()));
    pTrackMetadata->refAlbumInfo().setTitle(toQString(tag.album()));

    if ((readMask & READ_TAG_OMIT_COMMENT) == 0) {
        pTrackMetadata->refTrackInfo().setComment(toQString(tag.comment()));
    }

    int iYear = tag.year();
    if (iYear > 0) {
        pTrackMetadata->refTrackInfo().setYear(QString::number(iYear));
    }

    int iTrack = tag.track();
    if (iTrack > 0) {
        pTrackMetadata->refTrackInfo().setTrackNumber(QString::number(iTrack));
    }
}

void importTrackMetadataFromID3v2Tag(
        TrackMetadata* pTrackMetadata,
        const TagLib::ID3v2::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    const TagLib::ID3v2::Header* pHeader = tag.header();
    DEBUG_ASSERT(pHeader);
    if (!checkID3v2HeaderVersionSupported(*pHeader)) {
        kLogger.warning() << "Legacy ID3v2 version - importing only basic tags";
        importTrackMetadataFromTag(pTrackMetadata, tag);
        return; // done
    }

    // Omit to read comments with the default implementation provided by
    // TagLib. We are only interested in a CommentsFrame with an empty
    // description (see below). If no such CommentsFrame exists TagLib
    // arbitrarily picks the first one with a description that it finds,
    // e.g. "iTunNORM" or "iTunPGAP" with unexpected results for the user.
    // See also: https://bugs.launchpad.net/mixxx/+bug/1742617
    importTrackMetadataFromTag(pTrackMetadata, tag, READ_TAG_OMIT_COMMENT);

    TagLib::ID3v2::CommentsFrame* pCommentsFrame =
            findFirstCommentsFrameWithoutDescription(tag);
    if (pCommentsFrame) {
        pTrackMetadata->refTrackInfo().setComment(toQString(*pCommentsFrame));
    } else {
        // Compatibility workaround: ffmpeg 3.1.x maps DESCRIPTION fields of
        // FLAC files with Vorbis Tags into TXXX frames labeled "comment"
        // upon conversion to MP3. This might also happen when transcoding
        // other file types to MP3 if ffmpeg is writing comments into this
        // non-standard ID3v2 text frame.
        // Note: The description string that identifies certain text frames
        // is case-insensitive. We do the lookup with an upper-case string
        // like for all other frames.
        QString comment =
                readFirstUserTextIdentificationFrame(tag, "COMMENT");
        if (!comment.isNull()) {
            pTrackMetadata->refTrackInfo().setComment(comment);
        }
    }

    const TagLib::ID3v2::FrameList albumArtistFrame(tag.frameListMap()["TPE2"]);
    if (!albumArtistFrame.isEmpty()) {
        pTrackMetadata->refAlbumInfo().setArtist(toQStringFirstNotEmpty(albumArtistFrame));
    }

    if (pTrackMetadata->getAlbumInfo().getTitle().isEmpty()) {
        const TagLib::ID3v2::FrameList originalAlbumFrame(
                tag.frameListMap()["TOAL"]);
        pTrackMetadata->refAlbumInfo().setTitle(toQStringFirstNotEmpty(originalAlbumFrame));
    }

    const TagLib::ID3v2::FrameList composerFrame(tag.frameListMap()["TCOM"]);
    if (!composerFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setComposer(toQStringFirstNotEmpty(composerFrame));
    }

    const TagLib::ID3v2::FrameList groupingFrame(tag.frameListMap()["TIT1"]);
    if (!groupingFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setGrouping(toQStringFirstNotEmpty(groupingFrame));
    }

    // ID3v2.4.0: TDRC replaces TYER + TDAT
    const QString recordingTime(
            toQStringFirstNotEmpty(tag.frameListMap()["TDRC"]));
    if ((tag.header()->majorVersion() >= 4) && !recordingTime.isEmpty()) {
            pTrackMetadata->refTrackInfo().setYear(recordingTime);
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
            pTrackMetadata->refTrackInfo().setYear(year);
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
        pTrackMetadata->refTrackInfo().setTrackNumber(trackNumber);
        pTrackMetadata->refTrackInfo().setTrackTotal(trackTotal);
    }

    const TagLib::ID3v2::FrameList bpmFrame(tag.frameListMap()["TBPM"]);
    if (!bpmFrame.isEmpty()) {
        parseBpm(pTrackMetadata, toQStringFirstNotEmpty(bpmFrame));
        double bpmValue = pTrackMetadata->getTrackInfo().getBpm().getValue();
        // Some software use (or used) to write decimated values without comma,
        // so the number reads as 1352 or 14525 when it is 135.2 or 145.25
        if (bpmValue < Bpm::kValueMin || bpmValue > 1000 * Bpm::kValueMax) {
            // Considered out of range, don't try to adjust it
            kLogger.warning()
                    << "Ignoring invalid bpm value"
                    << bpmValue;
            bpmValue = Bpm::kValueUndefined;
        } else {
            double bpmValueOriginal = bpmValue;
            DEBUG_ASSERT(Bpm::kValueUndefined <= Bpm::kValueMax);
            bool adjusted = false;
            while (bpmValue > Bpm::kValueMax) {
                double bpmValueAdjusted = bpmValue / 10;
                if (bpmValueAdjusted < bpmValue) {
                    bpmValue = bpmValueAdjusted;
                    adjusted = true;
                    continue;
                }
                // Ensure that the loop always terminates even for invalid
                // values like Inf and NaN!
                kLogger.warning()
                        << "Ignoring invalid bpm value"
                        << bpmValueOriginal;
                bpmValue = Bpm::kValueUndefined;
                break;
            }
            if (adjusted) {
                kLogger.info()
                        << "Adjusted bpm value from"
                        << bpmValueOriginal
                        << "to"
                        << bpmValue;
            }
        }
        if (bpmValue != Bpm::kValueUndefined) {
            pTrackMetadata->refTrackInfo().setBpm(Bpm(bpmValue));
        }
    }

    const TagLib::ID3v2::FrameList keyFrame(tag.frameListMap()["TKEY"]);
    if (!keyFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setKey(toQStringFirstNotEmpty(keyFrame));
    }

    QString trackGain =
            readFirstUserTextIdentificationFrame(tag, "REPLAYGAIN_TRACK_GAIN");
    if (!trackGain.isEmpty()) {
        parseTrackGain(pTrackMetadata, trackGain);
    }
    QString trackPeak =
            readFirstUserTextIdentificationFrame(tag, "REPLAYGAIN_TRACK_PEAK");
    if (!trackPeak.isEmpty()) {
        parseTrackPeak(pTrackMetadata, trackPeak);
    }

    QString albumGain =
            readFirstUserTextIdentificationFrame(tag, "REPLAYGAIN_ALBUM_GAIN");
    if (!albumGain.isEmpty()) {
        parseAlbumGain(pTrackMetadata, albumGain);
    }
    QString albumPeak =
            readFirstUserTextIdentificationFrame(tag, "REPLAYGAIN_ALBUM_PEAK");
    if (!albumPeak.isEmpty()) {
        parseAlbumPeak(pTrackMetadata, albumPeak);
    }

    QString trackArtistId =
            readFirstUserTextIdentificationFrame(tag, "MusicBrainz Artist Id");
    if (!trackArtistId.isNull()) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(QUuid(trackArtistId));
    }
    QString trackReleaseId =
            readFirstUserTextIdentificationFrame(tag, "MusicBrainz Release Track Id");
    if (!trackReleaseId.isNull()) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(QUuid(trackReleaseId));
    }
    QString albumArtistId =
            readFirstUserTextIdentificationFrame(tag, "MusicBrainz Album Artist Id");
    if (!albumArtistId.isNull()) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId =
            readFirstUserTextIdentificationFrame(tag, "MusicBrainz Album Id");
    if (!albumReleaseId.isNull()) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId =
            readFirstUserTextIdentificationFrame(tag, "MusicBrainz Release Group Id");
    if (!albumReleaseGroupId.isNull()) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    const TagLib::ID3v2::FrameList conductorFrame(tag.frameListMap()["TPE3"]);
    if (!conductorFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setConductor(toQStringFirstNotEmpty(conductorFrame));
    }
    const TagLib::ID3v2::FrameList isrcFrame(tag.frameListMap()["TSRC"]);
    if (!isrcFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setISRC(toQStringFirstNotEmpty(isrcFrame));
    }
    const TagLib::ID3v2::FrameList languageFrame(tag.frameListMap()["TLAN"]);
    if (!languageFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setLanguage(toQStringFirstNotEmpty(languageFrame));
    }
    const TagLib::ID3v2::FrameList lyricistFrame(tag.frameListMap()["TEXT"]);
    if (!lyricistFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setLyricist(toQStringFirstNotEmpty(lyricistFrame));
    }
    if (tag.header()->majorVersion() >= 4) {
        const TagLib::ID3v2::FrameList moodFrame(tag.frameListMap()["TMOO"]);
        if (!moodFrame.isEmpty()) {
            pTrackMetadata->refTrackInfo().setMood(toQStringFirstNotEmpty(moodFrame));
        }
    }
    const TagLib::ID3v2::FrameList recordLabelFrame(tag.frameListMap()["TPUB"]);
    if (!recordLabelFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setRecordLabel(toQStringFirstNotEmpty(recordLabelFrame));
    }
    const TagLib::ID3v2::FrameList remixerFrame(tag.frameListMap()["TPE4"]);
    if (!remixerFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setRemixer(toQStringFirstNotEmpty(remixerFrame));
    }
    const TagLib::ID3v2::FrameList subtitleFrame(tag.frameListMap()["TIT3"]);
    if (!subtitleFrame.isEmpty()) {
        pTrackMetadata->refTrackInfo().setSubtitle(toQStringFirstNotEmpty(subtitleFrame));
    }
}

void importTrackMetadataFromAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    importTrackMetadataFromTag(pTrackMetadata, tag);

    // NOTE(uklotzde, 2018-01-28, https://bugs.launchpad.net/mixxx/+bug/1745847)
    // It turns out that the keys for APEv2 tags are case-sensitive and
    // some tag editors seem to write UPPERCASE Vorbis keys instead of
    // the CamelCase APEv2 keys suggested by the Picard Mapping table:
    // https://picard.musicbrainz.org/docs/mappings/

    QString albumArtist;
    if (readAPEItem(tag, "Album Artist", &albumArtist) ||
            readAPEItem(tag, "ALBUM ARTIST", &albumArtist) ||
            readAPEItem(tag, "ALBUMARTIST", &albumArtist)) {
        pTrackMetadata->refAlbumInfo().setArtist(albumArtist);
    }

    QString composer;
    if (readAPEItem(tag, "Composer", &composer) ||
            readAPEItem(tag, "COMPOSER", &composer)) {
        pTrackMetadata->refTrackInfo().setComposer(composer);
    }

    QString grouping;
    if (readAPEItem(tag, "Grouping", &grouping) ||
            readAPEItem(tag, "GROUPING", &grouping)) {
        pTrackMetadata->refTrackInfo().setGrouping(grouping);
    }

    // The release date (ISO 8601 without 'T' separator between date and time)
    // according to the mapping used by MusicBrainz Picard.
    // http://wiki.hydrogenaud.io/index.php?title=APE_date
    // https://picard.musicbrainz.org/docs/mappings
    QString year;
    if (readAPEItem(tag, "Year", &year) ||
            readAPEItem(tag, "YEAR", &year)) {
        pTrackMetadata->refTrackInfo().setYear(year);
    }

    QString trackNumber;
    if (readAPEItem(tag, "Track", &trackNumber) ||
            readAPEItem(tag, "TRACK", &trackNumber)) {
        QString trackTotal;
        TrackNumbers::splitString(
                trackNumber,
                &trackNumber,
                &trackTotal);
        pTrackMetadata->refTrackInfo().setTrackNumber(trackNumber);
        pTrackMetadata->refTrackInfo().setTrackTotal(trackTotal);
    }

    QString bpm;
    if (readAPEItem(tag, "BPM", &bpm)) {
        parseBpm(pTrackMetadata, bpm);
    }

    QString trackGain;
    if (readAPEItem(tag, "REPLAYGAIN_TRACK_GAIN", &trackGain)) {
        parseTrackGain(pTrackMetadata, trackGain);
    }
    QString trackPeak;
    if (readAPEItem(tag, "REPLAYGAIN_TRACK_PEAK", &trackPeak)) {
        parseTrackPeak(pTrackMetadata, trackPeak);
    }

    QString albumGain;
    if (readAPEItem(tag, "REPLAYGAIN_ALBUM_GAIN", &albumGain)) {
        parseTrackGain(pTrackMetadata, albumGain);
    }
    QString albumPeak;
    if (readAPEItem(tag, "REPLAYGAIN_ALBUM_PEAK", &albumPeak)) {
        parseAlbumPeak(pTrackMetadata, albumPeak);
    }

    QString trackArtistId;
    if (readAPEItem(tag, "MUSICBRAINZ_ARTISTID", &trackArtistId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(QUuid(trackArtistId));
    }
    QString trackReleaseId;
    if (readAPEItem(tag, "MUSICBRAINZ_RELEASETRACKID", &trackReleaseId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(QUuid(trackReleaseId));
    }
    QString albumArtistId;
    if (readAPEItem(tag, "MUSICBRAINZ_ALBUMARTISTID", &albumArtistId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId;
    if (readAPEItem(tag, "MUSICBRAINZ_ALBUMID", &albumReleaseId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId;
    if (readAPEItem(tag, "MUSICBRAINZ_RELEASEGROUPID", &albumReleaseGroupId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    QString conductor;
    if (readAPEItem(tag, "Conductor", &conductor) ||
            readAPEItem(tag, "CONDUCTOR", &conductor)) {
        pTrackMetadata->refTrackInfo().setConductor(conductor);
    }
    QString isrc;
    if (readAPEItem(tag, "ISRC", &isrc)) {
        pTrackMetadata->refTrackInfo().setISRC(isrc);
    }
    QString language;
    if (readAPEItem(tag, "Language", &language) ||
            readAPEItem(tag, "LANGUAGE", &language)) {
        pTrackMetadata->refTrackInfo().setLanguage(language);
    }
    QString lyricist;
    if (readAPEItem(tag, "Lyricist", &lyricist) ||
            readAPEItem(tag, "LYRICIST", &lyricist)) {
        pTrackMetadata->refTrackInfo().setLyricist(lyricist);
    }
    QString mood;
    if (readAPEItem(tag, "Mood", &mood) ||
            readAPEItem(tag, "MOOD", &mood)) {
        pTrackMetadata->refTrackInfo().setMood(mood);
    }
    QString remixer;
    if (readAPEItem(tag, "MixArtist", &remixer) ||
            readAPEItem(tag, "MIXARTIST", &remixer) ||
            readAPEItem(tag, "REMIXER", &remixer)) {
        pTrackMetadata->refTrackInfo().setRemixer(remixer);
    }
    QString recordLabel;
    if (readAPEItem(tag, "Label", &recordLabel) ||
            readAPEItem(tag, "LABEL", &recordLabel)) {
        pTrackMetadata->refTrackInfo().setRecordLabel(recordLabel);
    }
    QString subtitle;
    if (readAPEItem(tag, "Subtitle", &subtitle) ||
            readAPEItem(tag, "SUBTITLE", &subtitle)) {
        pTrackMetadata->refTrackInfo().setSubtitle(subtitle);
    }
}

void importTrackMetadataFromVorbisCommentTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    // Omit to read comments with the default implementation provided
    // by TagLib. The implementation is inconsistent with the handling
    // proposed by MusicBrainz (see below).
    importTrackMetadataFromTag(pTrackMetadata, tag, READ_TAG_OMIT_COMMENT);

    // The original specification only defines a "DESCRIPTION" field,
    // while MusicBrainz recommends to use "COMMENT". Mixxx follows
    // MusicBrainz.
    // http://www.xiph.org/vorbis/doc/v-comment.html
    // https://picard.musicbrainz.org/docs/mappings
    //
    // We are not relying on  TagLib (1.11.1) with a somehow inconsistent
    // handling. It prefers "DESCRIPTION" for reading, but adds a "COMMENT"
    // field upon writing when no "DESCRIPTION" field exists.
    QString comment;
    if (!readXiphCommentField(tag, "COMMENT", &comment) || comment.isEmpty()) {
        // Fallback to the the original "DESCRIPTION" field only if the
        // "COMMENT" field is either missing or empty
        readXiphCommentField(tag, "DESCRIPTION", &comment);
    }
    pTrackMetadata->refTrackInfo().setComment(comment);

    QString albumArtist;
    if (readXiphCommentField(tag, "ALBUMARTIST", &albumArtist) || // recommended field
            readXiphCommentField(tag, "ALBUM_ARTIST", &albumArtist) || // alternative field (with underscore character)
            readXiphCommentField(tag, "ALBUM ARTIST", &albumArtist) || // alternative field (with space character)
            readXiphCommentField(tag, "ENSEMBLE", &albumArtist)) { // alternative field
        pTrackMetadata->refAlbumInfo().setArtist(albumArtist);
    }

    QString composer;
    if (readXiphCommentField(tag, "COMPOSER", &composer)) {
        pTrackMetadata->refTrackInfo().setComposer(composer);
    }

    QString grouping;
    if (readXiphCommentField(tag, "GROUPING", &grouping)) {
        pTrackMetadata->refTrackInfo().setGrouping(grouping);
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
        pTrackMetadata->refTrackInfo().setTrackNumber(trackNumber);
        pTrackMetadata->refTrackInfo().setTrackTotal(trackTotal);
    }

    // The release date formatted according to ISO 8601. Might
    // be followed by a space character and arbitrary text.
    // http://age.hobba.nl/audio/mirroredpages/ogg-tagging.html
    QString date;
    if (readXiphCommentField(tag, "DATE", &date)) {
        pTrackMetadata->refTrackInfo().setYear(date);
    }

    // MusicBrainz recommends "BPM": https://picard.musicbrainz.org/docs/mappings
    // Mixxx (<= 2.0) favored "TEMPO": https://www.mixxx.org/wiki/doku.php/library_metadata_rewrite_using_taglib
    QString bpm;
    if (!readXiphCommentField(tag, "BPM", &bpm) || !parseBpm(pTrackMetadata, bpm)) {
        if (readXiphCommentField(tag, "TEMPO", &bpm)) {
            parseBpm(pTrackMetadata, bpm);
        }
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
        pTrackMetadata->refTrackInfo().setKey(key);
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

    QString albumGain;
    if (readXiphCommentField(tag, "REPLAYGAIN_ALBUM_GAIN", &albumGain)) {
        parseAlbumGain(pTrackMetadata, albumGain);
    }
    QString albumPeak;
    if (readXiphCommentField(tag, "REPLAYGAIN_ALBUM_PEAK", &albumPeak)) {
        parseAlbumPeak(pTrackMetadata, albumPeak);
    }

    QString trackArtistId;
    if (readXiphCommentField(tag, "MUSICBRAINZ_ARTISTID", &trackArtistId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(trackArtistId);
    }
    QString trackReleaseId;
    if (readXiphCommentField(tag, "MUSICBRAINZ_RELEASETRACKID", &trackReleaseId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(trackReleaseId);
    }
    QString albumArtistId;
    if (readXiphCommentField(tag, "MUSICBRAINZ_ALBUMARTISTID", &albumArtistId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId;
    if (readXiphCommentField(tag, "MUSICBRAINZ_ALBUMID", &albumReleaseId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId;
    if (readXiphCommentField(tag, "MUSICBRAINZ_RELEASEGROUPID", &albumReleaseGroupId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    QString conductor;
    if (readXiphCommentField(tag, "CONDUCTOR", &conductor)) {
        pTrackMetadata->refTrackInfo().setConductor(conductor);
    }
    QString isrc;
    if (readXiphCommentField(tag, "ISRC", &isrc)) {
        pTrackMetadata->refTrackInfo().setISRC(isrc);
    }
    QString language;
    if (readXiphCommentField(tag, "LANGUAGE", &language)) {
        pTrackMetadata->refTrackInfo().setLanguage(language);
    }
    QString lyricist;
    if (readXiphCommentField(tag, "LYRICIST", &lyricist)) {
        pTrackMetadata->refTrackInfo().setLyricist(lyricist);
    }
    QString mood;
    if (readXiphCommentField(tag, "MOOD", &mood)) {
        pTrackMetadata->refTrackInfo().setMood(mood);
    }
    QString recordLabel;
    if (readXiphCommentField(tag, "LABEL", &recordLabel)) {
        pTrackMetadata->refTrackInfo().setRecordLabel(recordLabel);
    }
    QString remixer;
    if (readXiphCommentField(tag, "REMIXER", &remixer)) {
        pTrackMetadata->refTrackInfo().setRemixer(remixer);
    }
    QString subtitle;
    if (readXiphCommentField(tag, "SUBTITLE", &subtitle)) {
        pTrackMetadata->refTrackInfo().setSubtitle(subtitle);
    }
}

void importTrackMetadataFromMP4Tag(TrackMetadata* pTrackMetadata, const TagLib::MP4::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    importTrackMetadataFromTag(pTrackMetadata, tag);

    QString albumArtist;
    if (readMP4Atom(tag, "aART", &albumArtist)) {
        pTrackMetadata->refAlbumInfo().setArtist(albumArtist);
    }

    QString composer;
    if (readMP4Atom(tag, "\251wrt", &composer)) {
        pTrackMetadata->refTrackInfo().setComposer(composer);
    }

    QString grouping;
    if (readMP4Atom(tag, "\251grp", &grouping)) {
        pTrackMetadata->refTrackInfo().setGrouping(grouping);
    }

    QString year;
    if (readMP4Atom(tag, "\251day", &year)) {
        pTrackMetadata->refTrackInfo().setYear(year);
    }

    // Read track number/total pair
    if (getItemListMap(tag).contains("trkn")) {
        const TagLib::MP4::Item trknItem = getItemListMap(tag)["trkn"];
        const TagLib::MP4::Item::IntPair trknPair = trknItem.toIntPair();
        const TrackNumbers trackNumbers(trknPair.first, trknPair.second);
        QString trackNumber;
        QString trackTotal;
        trackNumbers.toStrings(&trackNumber, &trackTotal);
        pTrackMetadata->refTrackInfo().setTrackNumber(trackNumber);
        pTrackMetadata->refTrackInfo().setTrackTotal(trackTotal);
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
        const TagLib::MP4::Item tmpoItem = getItemListMap(tag)["tmpo"];
        double bpmValue = tmpoItem.toInt();
        if (Bpm::isValidValue(bpmValue)) {
            pTrackMetadata->refTrackInfo().setBpm(Bpm(bpmValue));
        }
    }

    QString key;
    if (readMP4Atom(tag, "----:com.apple.iTunes:initialkey", &key) || // preferred (conforms to MixedInKey, Serato, Traktor)
            readMP4Atom(tag, "----:com.apple.iTunes:KEY", &key)) { // alternative (conforms to Rapid Evolution)
        pTrackMetadata->refTrackInfo().setKey(key);
    }

    QString trackGain;
    if (readMP4Atom(tag, "----:com.apple.iTunes:replaygain_track_gain", &trackGain)) {
        parseTrackGain(pTrackMetadata, trackGain);
    }
    QString trackPeak;
    if (readMP4Atom(tag, "----:com.apple.iTunes:replaygain_track_peak", &trackPeak)) {
        parseTrackPeak(pTrackMetadata, trackPeak);
    }

    QString albumGain;
    if (readMP4Atom(tag, "----:com.apple.iTunes:replaygain_album_gain", &albumGain)) {
        parseAlbumGain(pTrackMetadata, albumGain);
    }
    QString albumPeak;
    if (readMP4Atom(tag, "----:com.apple.iTunes:replaygain_album_peak", &albumPeak)) {
        parseAlbumPeak(pTrackMetadata, albumPeak);
    }

    QString trackArtistId;
    if (readMP4Atom(tag, "----:com.apple.iTunes:MusicBrainz Artist Id", &trackArtistId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(trackArtistId);
    }
    QString trackReleaseId;
    if (readMP4Atom(tag, "----:com.apple.iTunes:MusicBrainz Release Track Id", &trackReleaseId)) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(trackReleaseId);
    }
    QString albumArtistId;
    if (readMP4Atom(tag, "----:com.apple.iTunes:MusicBrainz Album Artist Id", &albumArtistId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId;
    if (readMP4Atom(tag, "----:com.apple.iTunes:MusicBrainz Album Id", &albumReleaseId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId;
    if (readMP4Atom(tag, "----:com.apple.iTunes:MusicBrainz Release Group Id", &albumReleaseGroupId)) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    QString conductor;
    if (readMP4Atom(tag, "----:com.apple.iTunes:CONDUCTOR", &conductor)) {
        pTrackMetadata->refTrackInfo().setConductor(conductor);
    }
    QString isrc;
    if (readMP4Atom(tag, "----:com.apple.iTunes:ISRC", &isrc)) {
        pTrackMetadata->refTrackInfo().setISRC(isrc);
    }
    QString language;
    if (readMP4Atom(tag, "----:com.apple.iTunes:LANGUAGE", &language)) {
        pTrackMetadata->refTrackInfo().setLanguage(language);
    }
    QString lyricist;
    if (readMP4Atom(tag, "----:com.apple.iTunes:LYRICIST", &lyricist)) {
        pTrackMetadata->refTrackInfo().setLyricist(lyricist);
    }
    QString mood;
    if (readMP4Atom(tag, "----:com.apple.iTunes:MOOD", &mood)) {
        pTrackMetadata->refTrackInfo().setMood(mood);
    }
    QString recordLabel;
    if (readMP4Atom(tag, "----:com.apple.iTunes:LABEL", &recordLabel)) {
        pTrackMetadata->refTrackInfo().setRecordLabel(recordLabel);
    }
    QString remixer;
    if (readMP4Atom(tag, "----:com.apple.iTunes:REMIXER", &remixer)) {
        pTrackMetadata->refTrackInfo().setRemixer(remixer);
    }
    QString subtitle;
    if (readMP4Atom(tag, "----:com.apple.iTunes:SUBTITLE", &subtitle)) {
        pTrackMetadata->refTrackInfo().setSubtitle(subtitle);
    }
}

void importTrackMetadataFromRIFFTag(TrackMetadata* pTrackMetadata, const TagLib::RIFF::Info::Tag& tag) {
    // Just delegate to the common import function
    importTrackMetadataFromTag(pTrackMetadata, tag);
}

void exportTrackMetadataIntoTag(
        TagLib::Tag* pTag,
        const TrackMetadata& trackMetadata,
        int writeMask) {
    DEBUG_ASSERT(pTag); // already validated before

    pTag->setArtist(toTagLibString(trackMetadata.getTrackInfo().getArtist()));
    pTag->setTitle(toTagLibString(trackMetadata.getTrackInfo().getTitle()));
    pTag->setAlbum(toTagLibString(trackMetadata.getAlbumInfo().getTitle()));
    pTag->setGenre(toTagLibString(trackMetadata.getTrackInfo().getGenre()));

    // Using setComment() from TagLib::Tag might have undesirable
    // effects if the tag type supports multiple comment fields for
    // different purposes, e.g. ID3v2. In this case setting the
    // comment here should be omitted.
    if (0 == (writeMask & WRITE_TAG_OMIT_COMMENT)) {
        pTag->setComment(toTagLibString(trackMetadata.getTrackInfo().getComment()));
    }

    // Specialized write functions for tags derived from Taglib::Tag might
    // be able to write the complete string from trackMetadata.getTrackInfo().getYear()
    // into the corresponding field. In this case parsing the year string
    // here should be omitted.
    if (0 == (writeMask & WRITE_TAG_OMIT_YEAR)) {
        // Set the numeric year if available
        const QDate yearDate(
                TrackMetadata::parseDateTime(trackMetadata.getTrackInfo().getYear()).date());
        if (yearDate.isValid()) {
            pTag->setYear(yearDate.year());
        }
    }

    // The numeric track number in TagLib::Tag does not reflect the total
    // number of tracks! Specialized write functions for tags derived from
    // Taglib::Tag might be able to handle both trackMetadata.getTrackInfo().getTrackNumber()
    // and trackMetadata.getTrackInfo().getTrackTotal(). In this case parsing the track
    // number string here is useless and should be omitted.
    if (0 == (writeMask & WRITE_TAG_OMIT_TRACK_NUMBER)) {
        // Set the numeric track number if available
        TrackNumbers parsedTrackNumbers;
        const TrackNumbers::ParseResult parseResult =
                TrackNumbers::parseFromString(trackMetadata.getTrackInfo().getTrackNumber(), &parsedTrackNumbers);
        if (TrackNumbers::ParseResult::VALID == parseResult) {
            pTag->setTrack(parsedTrackNumbers.getActual());
        }
    }
}

bool exportTrackMetadataIntoID3v2Tag(TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    const TagLib::ID3v2::Header* pHeader = pTag->header();
    DEBUG_ASSERT(pHeader);
    if (!checkID3v2HeaderVersionSupported(*pHeader)) {
        kLogger.warning() << "Legacy ID3v2 version - exporting only basic tags";
        exportTrackMetadataIntoTag(pTag, trackMetadata, WRITE_TAG_OMIT_NONE);
        return true; // done
    }

    // NOTE(uklotzde): Setting the comment for ID3v2 tags does
    // not work as expected when using TagLib 1.9.1 and must
    // be skipped! Otherwise special purpose comment fields
    // with a description like "iTunSMPB" might be overwritten.
    // Mixxx implements a special case handling for ID3v2 comment
    // frames (see below)
    exportTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_TRACK_NUMBER | WRITE_TAG_OMIT_YEAR | WRITE_TAG_OMIT_COMMENT);

    // Writing the common comments frame has been omitted (see above)
    writeID3v2CommentsFrameWithoutDescription(pTag, trackMetadata.getTrackInfo().getComment());

    writeID3v2TextIdentificationFrame(pTag, "TRCK",
            TrackNumbers::joinStrings(
                    trackMetadata.getTrackInfo().getTrackNumber(),
                    trackMetadata.getTrackInfo().getTrackTotal()));

    // NOTE(uklotz): Need to overwrite the TDRC frame if it
    // already exists. TagLib (1.9.x) writes a TDRC frame
    // even for ID3v2.3.0 tags if the numeric year is set.
    if ((pHeader->majorVersion() >= 4) || !pTag->frameList("TDRC").isEmpty()) {
        writeID3v2TextIdentificationFrame(pTag, "TDRC",
                trackMetadata.getTrackInfo().getYear());
    }
    if (pHeader->majorVersion() < 4) {
        // Fallback to TYER + TDAT
        const QDate date(TrackMetadata::parseDate(trackMetadata.getTrackInfo().getYear()));
        if (date.isValid()) {
            // Valid date
            writeID3v2TextIdentificationFrame(pTag, "TYER", date.toString(ID3V2_TYER_FORMAT), true);
            writeID3v2TextIdentificationFrame(pTag, "TDAT", date.toString(ID3V2_TDAT_FORMAT), true);
        } else {
            // Fallback to calendar year
            bool calendarYearValid = false;
            const QString calendarYear(TrackMetadata::formatCalendarYear(trackMetadata.getTrackInfo().getYear(), &calendarYearValid));
            if (calendarYearValid) {
                writeID3v2TextIdentificationFrame(pTag, "TYER", calendarYear, true);
            }
        }
    }

    writeID3v2TextIdentificationFrame(pTag, "TPE2",
            trackMetadata.getAlbumInfo().getArtist());
    writeID3v2TextIdentificationFrame(pTag, "TCOM",
            trackMetadata.getTrackInfo().getComposer());
    writeID3v2TextIdentificationFrame(pTag, "TIT1",
            trackMetadata.getTrackInfo().getGrouping());

    // According to the specification "The 'TBPM' frame contains the number
    // of beats per minute in the mainpart of the audio. The BPM is an
    // integer and represented as a numerical string."
    // Reference: http://id3.org/id3v2.3.0
    writeID3v2TextIdentificationFrame(pTag, "TBPM",
            formatBpmInteger(trackMetadata), true);

    writeID3v2TextIdentificationFrame(pTag, "TKEY", trackMetadata.getTrackInfo().getKey());

    writeID3v2UserTextIdentificationFrame(
            pTag,
            "REPLAYGAIN_TRACK_GAIN",
            formatTrackGain(trackMetadata),
            true);
    // NOTE(uklotzde, 2018-04-22): The analyzers currently doesn't
    // calculate a peak value, so leave it untouched in the file if
    // the value is invalid/absent. Otherwise the ID3 frame would
    // be deleted.
    if (hasTrackPeak(trackMetadata)) {
        writeID3v2UserTextIdentificationFrame(
                pTag,
                "REPLAYGAIN_TRACK_PEAK",
                formatTrackPeak(trackMetadata),
                true);
    }

    // TODO(XXX): The following tags are currently not stored in the
    // Mixxx library. Only write properties that have non-null values
    // to prevent deleting existing tags!

    if (hasAlbumGain(trackMetadata)) {
        writeID3v2UserTextIdentificationFrame(
                pTag,
                "REPLAYGAIN_ALBUM_GAIN",
                formatAlbumGain(trackMetadata),
                true);
    }
    if (hasAlbumPeak(trackMetadata)) {
        writeID3v2UserTextIdentificationFrame(
                pTag,
                "REPLAYGAIN_ALBUM_PEAK",
                formatAlbumPeak(trackMetadata),
                true);
    }

    if (!trackMetadata.getTrackInfo().getMusicBrainzArtistId().isNull()) {
        writeID3v2UserTextIdentificationFrame(
                pTag,
                "MusicBrainz Artist Id",
                trackMetadata.getTrackInfo().getMusicBrainzArtistId().toString(),
                false);
    }
    if (!trackMetadata.getTrackInfo().getMusicBrainzReleaseId().isNull()) {
        writeID3v2UserTextIdentificationFrame(
                pTag,
                "MusicBrainz Release Track Id",
                trackMetadata.getTrackInfo().getMusicBrainzReleaseId().toString(),
                false);
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzArtistId().isNull()) {
        writeID3v2UserTextIdentificationFrame(
                pTag,
                "MusicBrainz Album Artist Id",
                trackMetadata.getAlbumInfo().getMusicBrainzArtistId().toString(),
                false);
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzReleaseId().isNull()) {
        writeID3v2UserTextIdentificationFrame(
                pTag,
                "MusicBrainz Album Id",
                trackMetadata.getAlbumInfo().getMusicBrainzReleaseId().toString(),
                false);
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().isNull()) {
        writeID3v2UserTextIdentificationFrame(
                pTag,
                "MusicBrainz Release Group Id",
                trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().toString(),
                false);
    }

    writeID3v2TextIdentificationFrameStringIfNotNull(
            pTag,
            "TPE3",
            trackMetadata.getTrackInfo().getConductor());
    writeID3v2TextIdentificationFrameStringIfNotNull(
            pTag,
            "TSRC",
            trackMetadata.getTrackInfo().getISRC());
    writeID3v2TextIdentificationFrameStringIfNotNull(
            pTag,
            "TLAN",
            trackMetadata.getTrackInfo().getLanguage());
    writeID3v2TextIdentificationFrameStringIfNotNull(
            pTag,
            "TEXT",
            trackMetadata.getTrackInfo().getLyricist());
    if (pHeader->majorVersion() >= 4) {
        writeID3v2TextIdentificationFrameStringIfNotNull(
                pTag,
                "TMOO",
                trackMetadata.getTrackInfo().getMood());
    }
    writeID3v2TextIdentificationFrameStringIfNotNull(
            pTag,
            "TPUB",
            trackMetadata.getTrackInfo().getRecordLabel());
    writeID3v2TextIdentificationFrameStringIfNotNull(
            pTag,
            "TPE4",
            trackMetadata.getTrackInfo().getRemixer());
    writeID3v2TextIdentificationFrameStringIfNotNull(
            pTag,
            "TIT3",
            trackMetadata.getTrackInfo().getSubtitle());

    return true;
}

bool exportTrackMetadataIntoAPETag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    exportTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_TRACK_NUMBER | WRITE_TAG_OMIT_YEAR);

    // NOTE(uklotzde): Overwrite the numeric track number in the common
    // part of the tag with the custom string from the track metadata
    // (pass-through without any further validation)
    writeAPEItem(pTag, "Track",
            toTagLibString(TrackNumbers::joinStrings(
                    trackMetadata.getTrackInfo().getTrackNumber(),
                    trackMetadata.getTrackInfo().getTrackTotal())));

    writeAPEItem(pTag, "Year",
            toTagLibString(trackMetadata.getTrackInfo().getYear()));

    writeAPEItem(pTag, "Album Artist",
            toTagLibString(trackMetadata.getAlbumInfo().getArtist()));
    writeAPEItem(pTag, "Composer",
            toTagLibString(trackMetadata.getTrackInfo().getComposer()));
    writeAPEItem(pTag, "Grouping",
            toTagLibString(trackMetadata.getTrackInfo().getGrouping()));

    writeAPEItem(pTag, "BPM",
            toTagLibString(formatBpm(trackMetadata)));

    writeAPEItem(pTag, "INITIALKEY",
            toTagLibString(trackMetadata.getTrackInfo().getKey()));

    writeAPEItem(pTag, "REPLAYGAIN_TRACK_GAIN",
            toTagLibString(formatTrackGain(trackMetadata)));
    // NOTE(uklotzde, 2018-04-22): The analyzers currently doesn't
    // calculate a peak value, so leave it untouched in the file if
    // the value is invalid/absent. Otherwise the APE item would be
    // deleted.
    if (hasTrackPeak(trackMetadata)) {
        writeAPEItem(pTag, "REPLAYGAIN_TRACK_PEAK",
                toTagLibString(formatTrackPeak(trackMetadata)));
    }

    // TODO(XXX): The following tags are currently not stored in the
    // Mixxx library. Only write properties that have non-null values
    // to prevent deleting existing tags!

    if (hasAlbumGain(trackMetadata)) {
        writeAPEItem(pTag, "REPLAYGAIN_ALBUM_GAIN",
                toTagLibString(formatAlbumGain(trackMetadata)));
    }
    if (hasAlbumPeak(trackMetadata)) {
        writeAPEItem(pTag, "REPLAYGAIN_ALBUM_PEAK",
                toTagLibString(formatAlbumPeak(trackMetadata)));
    }

    if (!trackMetadata.getTrackInfo().getMusicBrainzArtistId().isNull()) {
        writeAPEItem(pTag, "MUSICBRAINZ_ARTISTID",
                toTagLibString(trackMetadata.getTrackInfo().getMusicBrainzArtistId().toString()));
    }
    if (!trackMetadata.getTrackInfo().getMusicBrainzReleaseId().isNull()) {
        writeAPEItem(pTag, "MUSICBRAINZ_RELEASETRACKID",
                toTagLibString(trackMetadata.getTrackInfo().getMusicBrainzReleaseId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzArtistId().isNull()) {
        writeAPEItem(pTag, "MUSICBRAINZ_ALBUMARTISTID",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzArtistId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzReleaseId().isNull()) {
        writeAPEItem(pTag, "MUSICBRAINZ_ALBUMID",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().isNull()) {
        writeAPEItem(pTag, "MUSICBRAINZ_RELEASEGROUPID",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().toString()));
    }

    if (!trackMetadata.getTrackInfo().getConductor().isNull()) {
        writeAPEItem(pTag, "Conductor",
                toTagLibString(trackMetadata.getTrackInfo().getConductor()));
    }
    if (!trackMetadata.getTrackInfo().getISRC().isNull()) {
        writeAPEItem(pTag, "ISRC",
                toTagLibString(trackMetadata.getTrackInfo().getISRC()));
    }
    if (!trackMetadata.getTrackInfo().getLanguage().isNull()) {
        writeAPEItem(pTag, "Language",
                toTagLibString(trackMetadata.getTrackInfo().getLanguage()));
    }
    if (!trackMetadata.getTrackInfo().getLyricist().isNull()) {
        writeAPEItem(pTag, "Lyricist",
                toTagLibString(trackMetadata.getTrackInfo().getLyricist()));
    }
    if (!trackMetadata.getTrackInfo().getMood().isNull()) {
        writeAPEItem(pTag, "Mood",
                toTagLibString(trackMetadata.getTrackInfo().getMood()));
    }
    if (!trackMetadata.getTrackInfo().getRecordLabel().isNull()) {
        writeAPEItem(pTag, "Label",
                toTagLibString(trackMetadata.getTrackInfo().getRecordLabel()));
    }
    if (!trackMetadata.getTrackInfo().getRemixer().isNull()) {
        writeAPEItem(pTag, "MixArtist",
                toTagLibString(trackMetadata.getTrackInfo().getRemixer()));
    }
    if (!trackMetadata.getTrackInfo().getSubtitle().isNull()) {
        writeAPEItem(pTag, "Subtitle",
                toTagLibString(trackMetadata.getTrackInfo().getSubtitle()));
    }

    return true;
}

bool exportTrackMetadataIntoXiphComment(TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    exportTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_TRACK_NUMBER |
            WRITE_TAG_OMIT_YEAR |
            WRITE_TAG_OMIT_COMMENT);

    // The original specification only defines a "DESCRIPTION" field,
    // while MusicBrainz recommends to use "COMMENT". Mixxx follows
    // MusicBrainz.
    // http://www.xiph.org/vorbis/doc/v-comment.html
    // https://picard.musicbrainz.org/docs/mappings
    if (hasXiphCommentField(*pTag, "COMMENT") || !hasXiphCommentField(*pTag, "DESCRIPTION")) {
        // MusicBrainz-style
        writeXiphCommentField(pTag, "COMMENT",
                toTagLibString(trackMetadata.getTrackInfo().getComment()));
    } else {
        // Preserve and update the "DESCRIPTION" field only if it already exists
        DEBUG_ASSERT(hasXiphCommentField(*pTag, "DESCRIPTION"));
        writeXiphCommentField(pTag, "DESCRIPTION",
                toTagLibString(trackMetadata.getTrackInfo().getComment()));
    }

    // Write unambiguous fields
    writeXiphCommentField(pTag, "DATE",
            toTagLibString(trackMetadata.getTrackInfo().getYear()));
    writeXiphCommentField(pTag, "COMPOSER",
            toTagLibString(trackMetadata.getTrackInfo().getComposer()));
    writeXiphCommentField(pTag, "GROUPING",
            toTagLibString(trackMetadata.getTrackInfo().getGrouping()));
    writeXiphCommentField(pTag, "TRACKNUMBER",
            toTagLibString(trackMetadata.getTrackInfo().getTrackNumber()));

    // According to https://wiki.xiph.org/Field_names "TRACKTOTAL" is
    // the proposed field name, but some applications use "TOTALTRACKS".
    const TagLib::String trackTotal(
            toTagLibString(trackMetadata.getTrackInfo().getTrackTotal()));
    writeXiphCommentField(pTag, "TRACKTOTAL", trackTotal); // recommended field
    updateXiphCommentField(pTag, "TOTALTRACKS", trackTotal); // alternative field

    const TagLib::String albumArtist(
            toTagLibString(trackMetadata.getAlbumInfo().getArtist()));
    writeXiphCommentField(pTag, "ALBUMARTIST", albumArtist); // recommended field
    updateXiphCommentField(pTag, "ALBUM_ARTIST", albumArtist); // alternative field
    updateXiphCommentField(pTag, "ALBUM ARTIST", albumArtist); // alternative field
    updateXiphCommentField(pTag, "ENSEMBLE", albumArtist); // alternative field

    const TagLib::String bpm(
            toTagLibString(formatBpm(trackMetadata)));
    // MusicBrainz recommends "BPM": https://picard.musicbrainz.org/docs/mappings
    // Mixxx (<= 2.0) favored "TEMPO": https://www.mixxx.org/wiki/doku.php/library_metadata_rewrite_using_taglib
    if (hasXiphCommentField(*pTag, "BPM") || !hasXiphCommentField(*pTag, "TEMPO")) {
        // Update or add the recommended field for BPM values
        writeXiphCommentField(pTag, "BPM", bpm);
    } else {
        // Update the legacy field for BPM values only if it already exists exclusively
        DEBUG_ASSERT(hasXiphCommentField(*pTag, "TEMPO"));
        writeXiphCommentField(pTag, "TEMPO", bpm);
    }

    // Write both INITIALKEY and KEY
    const TagLib::String key(
            toTagLibString(trackMetadata.getTrackInfo().getKey()));
    writeXiphCommentField(pTag, "INITIALKEY", key); // recommended field
    updateXiphCommentField(pTag, "KEY", key); // alternative field

    writeXiphCommentField(pTag, "REPLAYGAIN_TRACK_GAIN",
            toTagLibString(formatTrackGain(trackMetadata)));
    // NOTE(uklotzde, 2018-04-22): The analyzers currently doesn't
    // calculate a peak value, so leave it untouched in the file if
    // the value is invalid/absent. Otherwise the comment field would
    // be deleted.
    if (hasTrackPeak(trackMetadata)) {
        writeXiphCommentField(pTag, "REPLAYGAIN_TRACK_PEAK",
                toTagLibString(formatTrackPeak(trackMetadata)));
    }

    // TODO(XXX): The following tags are currently not stored in the
    // Mixxx library. Only write properties that have non-null values
    // to prevent deleting existing tags!

    if (hasAlbumGain(trackMetadata)) {
        writeXiphCommentField(pTag, "REPLAYGAIN_ALBUM_GAIN",
                toTagLibString(formatAlbumGain(trackMetadata)));
    }
    if (hasAlbumPeak(trackMetadata)) {
        writeXiphCommentField(pTag, "REPLAYGAIN_ALBUM_PEAK",
                toTagLibString(formatAlbumPeak(trackMetadata)));
    }
    if (!trackMetadata.getTrackInfo().getMusicBrainzArtistId().isNull()) {
        writeXiphCommentField(pTag, "MUSICBRAINZ_ARTISTID",
                toTagLibString(trackMetadata.getTrackInfo().getMusicBrainzArtistId().toString()));
    }
    if (!trackMetadata.getTrackInfo().getMusicBrainzReleaseId().isNull()) {
        writeXiphCommentField(pTag, "MUSICBRAINZ_RELEASETRACKID",
                toTagLibString(trackMetadata.getTrackInfo().getMusicBrainzReleaseId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzArtistId().isNull()) {
        writeXiphCommentField(pTag, "MUSICBRAINZ_ALBUMARTISTID",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzArtistId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzReleaseId().isNull()) {
        writeXiphCommentField(pTag, "MUSICBRAINZ_ALBUMID",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().isNull()) {
        writeXiphCommentField(pTag, "MUSICBRAINZ_RELEASEGROUPID",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().toString()));
    }
    if (!trackMetadata.getTrackInfo().getConductor().isNull()) {
        writeXiphCommentField(pTag, "CONDUCTOR",
                toTagLibString(trackMetadata.getTrackInfo().getConductor()));
    }
    if (!trackMetadata.getTrackInfo().getISRC().isNull()) {
        writeXiphCommentField(pTag, "ISRC",
                toTagLibString(trackMetadata.getTrackInfo().getISRC()));
    }
    if (!trackMetadata.getTrackInfo().getLanguage().isNull()) {
        writeXiphCommentField(pTag, "LANGUAGE",
                toTagLibString(trackMetadata.getTrackInfo().getLanguage()));
    }
    if (!trackMetadata.getTrackInfo().getLyricist().isNull()) {
        writeXiphCommentField(pTag, "LYRICIST",
                toTagLibString(trackMetadata.getTrackInfo().getLyricist()));
    }
    if (!trackMetadata.getTrackInfo().getMood().isNull()) {
        writeXiphCommentField(pTag, "MOOD",
                toTagLibString(trackMetadata.getTrackInfo().getMood()));
    }
    if (!trackMetadata.getTrackInfo().getRecordLabel().isNull()) {
        writeXiphCommentField(pTag, "LABEL",
                toTagLibString(trackMetadata.getTrackInfo().getRecordLabel()));
    }
    if (!trackMetadata.getTrackInfo().getRemixer().isNull()) {
        writeXiphCommentField(pTag, "REMIXER",
                toTagLibString(trackMetadata.getTrackInfo().getRemixer()));
    }
    if (!trackMetadata.getTrackInfo().getSubtitle().isNull()) {
        writeXiphCommentField(pTag, "SUBTITLE",
                toTagLibString(trackMetadata.getTrackInfo().getSubtitle()));
    }

    return true;
}

bool exportTrackMetadataIntoMP4Tag(TagLib::MP4::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    exportTrackMetadataIntoTag(pTag, trackMetadata,
            WRITE_TAG_OMIT_TRACK_NUMBER | WRITE_TAG_OMIT_YEAR);

    // Write track number/total pair
    TrackNumbers parsedTrackNumbers;
    const TrackNumbers::ParseResult parseResult =
            TrackNumbers::parseFromStrings(
                    trackMetadata.getTrackInfo().getTrackNumber(),
                    trackMetadata.getTrackInfo().getTrackTotal(),
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
        kLogger.warning() << "Invalid track numbers:"
            << TrackNumbers::joinStrings(
                    trackMetadata.getTrackInfo().getTrackNumber(),
                    trackMetadata.getTrackInfo().getTrackTotal());
    }

    writeMP4Atom(pTag, "\251day", toTagLibString(trackMetadata.getTrackInfo().getYear()));

    writeMP4Atom(pTag, "aART", toTagLibString(trackMetadata.getAlbumInfo().getArtist()));
    writeMP4Atom(pTag, "\251wrt", toTagLibString(trackMetadata.getTrackInfo().getComposer()));
    writeMP4Atom(pTag, "\251grp", toTagLibString(trackMetadata.getTrackInfo().getGrouping()));

    // Write both BPM fields (just in case)
    if (trackMetadata.getTrackInfo().getBpm().hasValue()) {
        // 16-bit integer value
        const int tmpoValue =
                Bpm::valueToInteger(trackMetadata.getTrackInfo().getBpm().getValue());
        pTag->itemListMap()["tmpo"] = tmpoValue;
    } else {
        pTag->itemListMap().erase("tmpo");
    }
    writeMP4Atom(pTag, "----:com.apple.iTunes:BPM",
            toTagLibString(formatBpm(trackMetadata)));

    const TagLib::String key =
            toTagLibString(trackMetadata.getTrackInfo().getKey());
    writeMP4Atom(pTag, "----:com.apple.iTunes:initialkey", key); // preferred
    updateMP4Atom(pTag, "----:com.apple.iTunes:KEY", key); // alternative

    writeMP4Atom(pTag, "----:com.apple.iTunes:replaygain_track_gain",
            toTagLibString(formatTrackGain(trackMetadata)));
    // NOTE(uklotzde, 2018-04-22): The analyzers currently doesn't
    // calculate a peak value, so leave it untouched in the file if
    // the value is invalid/absent. Otherwise the MP4 atom would be
    // deleted.
    if (hasTrackPeak(trackMetadata)) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:replaygain_track_peak",
                toTagLibString(formatTrackPeak(trackMetadata)));
    }

    // TODO(XXX): The following tags are currently not stored in the
    // Mixxx library. Only write properties that have non-null values
    // to prevent deleting existing tags!

    if (hasAlbumGain(trackMetadata)) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:replaygain_album_gain",
                toTagLibString(formatAlbumGain(trackMetadata)));
    }
    if (hasAlbumPeak(trackMetadata)) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:replaygain_album_peak",
                toTagLibString(formatAlbumPeak(trackMetadata)));
    }
    if (!trackMetadata.getTrackInfo().getMusicBrainzArtistId().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:MusicBrainz Artist Id",
                toTagLibString(trackMetadata.getTrackInfo().getMusicBrainzArtistId().toString()));
    }
    if (!trackMetadata.getTrackInfo().getMusicBrainzReleaseId().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:MusicBrainz Release Track Id",
                toTagLibString(trackMetadata.getTrackInfo().getMusicBrainzReleaseId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzArtistId().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:MusicBrainz Album Artist Id",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzArtistId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzReleaseId().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:MusicBrainz Album Id",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseId().toString()));
    }
    if (!trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:MusicBrainz Release Group Id",
                toTagLibString(trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId().toString()));
    }
    if (!trackMetadata.getTrackInfo().getConductor().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:CONDUCTOR",
                toTagLibString(trackMetadata.getTrackInfo().getConductor()));
    }
    if (!trackMetadata.getTrackInfo().getISRC().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:ISRC",
                toTagLibString(trackMetadata.getTrackInfo().getISRC()));
    }
    if (!trackMetadata.getTrackInfo().getLanguage().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:LANGUAGE",
                toTagLibString(trackMetadata.getTrackInfo().getLanguage()));
    }
    if (!trackMetadata.getTrackInfo().getLyricist().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:LYRICIST",
                toTagLibString(trackMetadata.getTrackInfo().getLyricist()));
    }
    if (!trackMetadata.getTrackInfo().getMood().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:MOOD",
                toTagLibString(trackMetadata.getTrackInfo().getMood()));
    }
    if (!trackMetadata.getTrackInfo().getRecordLabel().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:LABEL",
                toTagLibString(trackMetadata.getTrackInfo().getRecordLabel()));
    }
    if (!trackMetadata.getTrackInfo().getRemixer().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:REMIXER",
                toTagLibString(trackMetadata.getTrackInfo().getRemixer()));
    }
    if (!trackMetadata.getTrackInfo().getSubtitle().isNull()) {
        writeMP4Atom(pTag, "----:com.apple.iTunes:SUBTITLE",
                toTagLibString(trackMetadata.getTrackInfo().getSubtitle()));
    }

    return true;
}

} // namespace taglib

} //namespace mixxx
