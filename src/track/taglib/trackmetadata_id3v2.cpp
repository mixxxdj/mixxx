#include "track/taglib/trackmetadata_id3v2.h"

#include <taglib/attachedpictureframe.h>
#include <taglib/commentsframe.h>
#include <taglib/generalencapsulatedobjectframe.h>
#include <taglib/textidentificationframe.h>
#include <taglib/unknownframe.h>

#include <array>
#if defined(__EXTRA_METADATA__)
#include <taglib/uniquefileidentifierframe.h>
#endif // __EXTRA_METADATA__

#include "track/tracknumbers.h"
#include "util/logger.h"

namespace mixxx {

namespace {

Logger kLogger("TagLib");

} // anonymous namespace

namespace taglib {

namespace {

// Only ID3v2.3 and ID3v2.4 are supported for both importing and
// exporting text frames. ID3v2.2 uses different frame identifiers,
// i.e. only 3 instead of 4 characters.
// https://en.wikipedia.org/wiki/ID3#ID3v2
// http://id3.org/Developer%20Information
const unsigned int kMinVersion = 3;

bool checkHeaderVersionSupported(
        const TagLib::ID3v2::Header& header) {
    if (header.majorVersion() < kMinVersion) {
        kLogger.warning().noquote()
                << QString(QStringLiteral("ID3v2.%1 is only partially supported - please convert your file tags to at least ID3v2.%2"))
                           .arg(QString::number(header.majorVersion()), QString::number(kMinVersion));
        return false;
    } else {
        return true;
    }
}

// Preferred picture types for cover art sorted by priority
const std::array<TagLib::ID3v2::AttachedPictureFrame::Type, 4> kPreferredPictureTypes{{
        TagLib::ID3v2::AttachedPictureFrame::FrontCover,   // Front cover image of the album
        TagLib::ID3v2::AttachedPictureFrame::Media,        // Image from the album itself
        TagLib::ID3v2::AttachedPictureFrame::Illustration, // Illustration related to the track
        TagLib::ID3v2::AttachedPictureFrame::Other,
}};

// http://id3.org/id3v2.3.0
// "TYER: The 'Year' frame is a numeric string with a year of the
// recording. This frame is always four characters long (until
// the year 10000)."
const QString kFormatTYER = QStringLiteral("yyyy");

// http://id3.org/id3v2.3.0
// "TDAT:  The 'Date' frame is a numeric string in the DDMM
// format containing the date for the recording. This field
// is always four characters long."
const QString kFormatTDAT = QStringLiteral("ddMM");

// Owners of ID3v2 UFID frames.
// NOTE(uklotzde, 2019-09-28): This is the owner string for MusicBrainz
// as written by MusicBrainz Picard 2.1.3 although the mapping table
// doesn't mention any "http://" prefix.
// See also: https://picard.musicbrainz.org/docs/mappings
const QString kMusicBrainzOwner = QStringLiteral("http://musicbrainz.org");

// Serato frames
const QString kFrameDescriptionSeratoBeatGrid = QStringLiteral("Serato BeatGrid");
const QString kFrameDescriptionSeratoMarkers = QStringLiteral("Serato Markers_");
const QString kFrameDescriptionSeratoMarkers2 = QStringLiteral("Serato Markers2");

// Returns the text of an ID3v2 frame as a string.
inline QString frameToQString(
        const TagLib::ID3v2::Frame& frame) {
    return toQString(frame.toString());
}

// Returns the first frame of an ID3v2 tag as a string.
QString firstNonEmptyFrameToQString(
        const TagLib::ID3v2::FrameList& frameList) {
    for (const TagLib::ID3v2::Frame* pFrame : frameList) {
        if (pFrame) {
            TagLib::String str = pFrame->toString();
            if (!str.isEmpty()) {
                return toQString(str);
            }
            const auto* pUnknownFrame = dynamic_cast<const TagLib::ID3v2::UnknownFrame*>(pFrame);
            if (pUnknownFrame) {
                kLogger.warning()
                        << "Unsupported ID3v2 frame"
                        << pUnknownFrame->frameID().data();
            }
        }
    }
    return QString();
}

TagLib::String::Type getStringType(
        const TagLib::ID3v2::Tag& tag,
        bool isNumericOrURL = false) {
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
            it != commentsFrames.end();
            ++it) {
        auto* pFrame =
                dynamic_cast<TagLib::ID3v2::CommentsFrame*>(*it);
        if (pFrame) {
            const QString frameDescription(
                    toQString(pFrame->description()));
            if (0 == frameDescription.compare(description, Qt::CaseInsensitive)) {
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
// If multiple text frames with matching descriptions exist prefer the first
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
    for (TagLib::ID3v2::FrameList::ConstIterator it = textFrames.begin();
            it != textFrames.end();
            ++it) {
        auto* pFrame =
                dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
        if (pFrame) {
            const QString frameDescription = toQString(pFrame->description());
            if (0 == frameDescription.compare(description, Qt::CaseInsensitive)) {
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

#if defined(__EXTRA_METADATA__)
// Finds the first UFID frame that with a matching owner (case-insensitive).
// If multiple UFID frames with matching descriptions exist prefer the first
// with a non-empty content if requested.
TagLib::ID3v2::UniqueFileIdentifierFrame* findFirstUniqueFileIdentifierFrame(
        const TagLib::ID3v2::Tag& tag,
        const QString& owner,
        bool preferNotEmpty = true) {
    DEBUG_ASSERT(!owner.isEmpty());
    TagLib::ID3v2::UniqueFileIdentifierFrame* pFirstFrame = nullptr;
    // Bind the const-ref result to avoid a local copy
    const TagLib::ID3v2::FrameList& ufidFrames =
            tag.frameListMap()["UFID"];
    for (TagLib::ID3v2::FrameList::ConstIterator it = ufidFrames.begin();
            it != ufidFrames.end();
            ++it) {
        auto pFrame =
                dynamic_cast<TagLib::ID3v2::UniqueFileIdentifierFrame*>(*it);
        if (pFrame) {
            const QString frameOwner = toQString(pFrame->owner());
            if (0 == frameOwner.compare(owner, Qt::CaseInsensitive)) {
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

QByteArray readFirstUniqueFileIdentifierFrame(
        const TagLib::ID3v2::Tag& tag,
        const QString& owner) {
    const TagLib::ID3v2::UniqueFileIdentifierFrame* pFrame =
            findFirstUniqueFileIdentifierFrame(tag, owner);
    if (pFrame) {
        return QByteArray(pFrame->identifier().data(), pFrame->identifier().size());
    } else {
        return QByteArray();
    }
}
#endif // __EXTRA_METADATA__

// Finds the first GEOB frame that with a matching description (case-insensitive).
// If multiple GEOB frames with matching descriptions exist prefer the first
// with a non-empty content if requested.
TagLib::ID3v2::GeneralEncapsulatedObjectFrame* findFirstGeneralEncapsulatedObjectFrame(
        const TagLib::ID3v2::Tag& tag,
        const QString& description,
        bool preferNotEmpty = true) {
    DEBUG_ASSERT(!description.isEmpty());
    TagLib::ID3v2::GeneralEncapsulatedObjectFrame* pFirstFrame = nullptr;
    // Bind the const-ref result to avoid a local copy
    const TagLib::ID3v2::FrameList& geobFrames =
            tag.frameListMap()["GEOB"];
    for (TagLib::ID3v2::FrameList::ConstIterator it(geobFrames.begin());
            it != geobFrames.end();
            ++it) {
        auto* pFrame =
                dynamic_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(*it);
        if (pFrame) {
            const QString frameDescription(
                    toQString(pFrame->description()));
            if (0 == frameDescription.compare(description, Qt::CaseInsensitive)) {
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

inline QByteArray readFirstGeneralEncapsulatedObjectFrame(
        const TagLib::ID3v2::Tag& tag,
        const QString& description) {
    const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* pGeobFrame =
            findFirstGeneralEncapsulatedObjectFrame(tag, description);
    if (pGeobFrame) {
        return toQByteArrayRaw(pGeobFrame->object());
    } else {
        return QByteArray();
    }
}

void writeTextIdentificationFrame(
        TagLib::ID3v2::Tag* pTag,
        const TagLib::ByteVector& id,
        const QString& text,
        bool isNumericOrURL = false) {
    DEBUG_ASSERT(pTag);

    // Remove all existing frames before adding a new one
    pTag->removeFrames(id);
    if (!text.isEmpty()) {
        // Only add non-empty frames
        const TagLib::String::Type stringType =
                getStringType(*pTag, isNumericOrURL);
        auto pFrame =
                std::make_unique<TagLib::ID3v2::TextIdentificationFrame>(id, stringType);
        pFrame->setText(toTString(text));
        pTag->addFrame(pFrame.get());
        // Now that the plain pointer in pFrame is owned and managed by
        // pTag we need to release the ownership to avoid double deletion!
        pFrame.release();
    }
}

void writeUserTextIdentificationFrame(
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
            pFrame->setDescription(toTString(description));
            pFrame->setText(toTString(text));
        }
    } else {
        // Add a new (non-empty) frame
        if (!text.isEmpty()) {
            const TagLib::String::Type stringType =
                    getStringType(*pTag, isNumericOrURL);
            auto pFrame =
                    std::make_unique<TagLib::ID3v2::UserTextIdentificationFrame>(stringType);
            pFrame->setDescription(toTString(description));
            pFrame->setText(toTString(text));
            pTag->addFrame(pFrame.get());
            // Now that the plain pointer in pFrame is owned and managed by
            // pTag we need to release the ownership to avoid double deletion!
            pFrame.release();
        }
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
                it != textFrames.end();
                ++it) {
            auto* pFrame =
                    dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(*it);
            if (pFrame) {
                const QString frameDescription(
                        toQString(pFrame->description()));
                if (0 == frameDescription.compare(description, Qt::CaseInsensitive)) {
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

void writeCommentsFrame(
        TagLib::ID3v2::Tag* pTag,
        const TagLib::String& text,
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
            pFrame->setDescription(toTString(description));
            pFrame->setText(text);
        }
    } else {
        // Add a new (non-empty) frame
        if (!text.isEmpty()) {
            const TagLib::String::Type stringType =
                    getStringType(*pTag, isNumericOrURL);
            auto pFrame =
                    std::make_unique<TagLib::ID3v2::CommentsFrame>(stringType);
            pFrame->setDescription(toTString(description));
            pFrame->setText(text);
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
        kLogger.warning()
                << "Removed"
                << numberOfRemovedCommentFrames
                << "non-standard ID3v2 TXXX comment frames";
    }
}

void writeCommentsFrameWithoutDescription(
        TagLib::ID3v2::Tag* pTag,
        const TagLib::String& text,
        bool isNumericOrURL = false) {
    writeCommentsFrame(pTag, text, QString(), isNumericOrURL);
}

#if defined(__EXTRA_METADATA__)
void writeUniqueFileIdentifierFrame(
        TagLib::ID3v2::Tag* pTag,
        const QString& owner,
        const QByteArray& identifier) {
    TagLib::ID3v2::UniqueFileIdentifierFrame* pFrame =
            findFirstUniqueFileIdentifierFrame(*pTag, owner);
    if (pFrame) {
        // Modify existing frame
        if (identifier.isEmpty()) {
            // Purge empty frames
            pTag->removeFrame(pFrame);
        } else {
            pFrame->setOwner(toTString(owner));
            pFrame->setIdentifier(TagLib::ByteVector(identifier.constData(), identifier.size()));
        }
    } else {
        // Add a new (non-empty) frame
        if (!identifier.isEmpty()) {
            auto pFrame =
                    std::make_unique<TagLib::ID3v2::UniqueFileIdentifierFrame>(
                            toTString(owner),
                            TagLib::ByteVector(identifier.constData(), identifier.size()));
            pTag->addFrame(pFrame.get());
            // Now that the plain pointer in pFrame is owned and managed by
            // pTag we need to release the ownership to avoid double deletion!
            pFrame.release();
        }
    }
}
#endif // __EXTRA_METADATA__

#if defined(__EXPORT_SERATO_MARKERS__)
void writeGeneralEncapsulatedObjectFrame(
        TagLib::ID3v2::Tag* pTag,
        const QString& description,
        const QByteArray& data) {
    TagLib::ID3v2::GeneralEncapsulatedObjectFrame* pFrame =
            findFirstGeneralEncapsulatedObjectFrame(*pTag, description);
    if (pFrame) {
        // Modify existing frame
        if (data.isEmpty()) {
            // Purge empty frames
            pTag->removeFrame(pFrame);
        } else {
            pFrame->setDescription(toTString(description));
            pFrame->setObject(toTByteVector(data));
        }
    } else {
        // Add a new (non-empty) frame
        if (!data.isEmpty()) {
            auto pFrame =
                    std::make_unique<TagLib::ID3v2::GeneralEncapsulatedObjectFrame>();
            pFrame->setDescription(toTString(description));
            pFrame->setObject(toTByteVector(data));
            pTag->addFrame(pFrame.get());
            // Now that the plain pointer in pFrame is owned and managed by
            // pTag we need to release the ownership to avoid double deletion!
            pFrame.release();
        }
    }
}
#endif // __EXPORT_SERATO_MARKERS__

template<typename T>
const T* downcastFrame(TagLib::ID3v2::Frame* frame) {
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

inline QImage loadImageFromPictureFrame(
        const TagLib::ID3v2::AttachedPictureFrame& apicFrame) {
    return loadImageFromByteVector(apicFrame.picture());
}

inline QString formatBpmInteger(
        const TrackMetadata& trackMetadata) {
    if (!trackMetadata.getTrackInfo().getBpm().hasValue()) {
        return QString();
    }
    return QString::number(
            Bpm::valueToInteger(
                    trackMetadata.getTrackInfo().getBpm().getValue()));
}

} // anonymous namespace

namespace id3v2 {

bool importCoverImageFromTag(
        QImage* pCoverArt,
        const TagLib::ID3v2::Tag& tag) {
    if (!pCoverArt) {
        return false; // nothing to do
    }

    const auto iterAPIC = tag.frameListMap().find("APIC");
    if ((iterAPIC == tag.frameListMap().end()) || iterAPIC->second.isEmpty()) {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "No cover art: None or empty list of ID3v2 APIC frames";
        }
        return false; // abort
    }

    const TagLib::ID3v2::FrameList pFrames = iterAPIC->second;
    for (const auto coverArtType : kPreferredPictureTypes) {
        for (auto* const pFrame : pFrames) {
            const auto* pApicFrame =
                    downcastFrame<TagLib::ID3v2::AttachedPictureFrame>(pFrame);
            if (pApicFrame && (pApicFrame->type() == coverArtType)) {
                QImage image(loadImageFromPictureFrame(*pApicFrame));
                if (image.isNull()) {
                    kLogger.warning()
                            << "Failed to load image from ID3v2 APIC frame of type"
                            << pApicFrame->type();
                    continue;
                } else {
                    *pCoverArt = image;
                    return true; // success
                }
            }
        }
    }

    // Fallback: No best match -> Simply select the 1st loadable image
    for (auto* const pFrame : pFrames) {
        const auto* pApicFrame =
                downcastFrame<TagLib::ID3v2::AttachedPictureFrame>(pFrame);
        if (pApicFrame) {
            const QImage image(loadImageFromPictureFrame(*pApicFrame));
            if (image.isNull()) {
                kLogger.warning()
                        << "Failed to load image from ID3v2 APIC frame of type"
                        << pApicFrame->type();
                continue;
            } else {
                *pCoverArt = image;
                return true; // success
            }
        }
    }

    if (kLogger.debugEnabled()) {
        kLogger.debug() << "No cover art found in ID3v2 tag";
    }
    return false;
}

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::ID3v2::Tag& tag) {
    if (!pTrackMetadata) {
        return; // nothing to do
    }

    const TagLib::ID3v2::Header* pHeader = tag.header();
    DEBUG_ASSERT(pHeader);
    if (!checkHeaderVersionSupported(*pHeader)) {
        kLogger.warning() << "Legacy ID3v2 version - importing only basic tags";
        taglib::importTrackMetadataFromTag(pTrackMetadata, tag);
        return; // done
    }

    // Omit to read comments with the default implementation provided by
    // TagLib. We are only interested in a CommentsFrame with an empty
    // description (see below). If no such CommentsFrame exists TagLib
    // arbitrarily picks the first one with a description that it finds,
    // e.g. "iTunNORM" or "iTunPGAP" with unexpected results for the user.
    // See also: https://bugs.launchpad.net/mixxx/+bug/1742617
    taglib::importTrackMetadataFromTag(
            pTrackMetadata,
            tag,
            ReadTagFlag::OmitComment);

    TagLib::ID3v2::CommentsFrame* pCommentsFrame =
            findFirstCommentsFrameWithoutDescription(tag);
    if (pCommentsFrame) {
        pTrackMetadata->refTrackInfo().setComment(
                frameToQString(*pCommentsFrame));
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
                readFirstUserTextIdentificationFrame(
                        tag,
                        QStringLiteral("COMMENT"));
        if (!comment.isNull()) {
            pTrackMetadata->refTrackInfo().setComment(comment);
        }
    }

    const TagLib::ID3v2::FrameList albumArtistFrames(tag.frameListMap()["TPE2"]);
    if (!albumArtistFrames.isEmpty()) {
        pTrackMetadata->refAlbumInfo().setArtist(
                firstNonEmptyFrameToQString(albumArtistFrames));
    }

    if (pTrackMetadata->getAlbumInfo().getTitle().isEmpty()) {
        // Use the original album title as a fallback
        const TagLib::ID3v2::FrameList originalAlbumFrames(
                tag.frameListMap()["TOAL"]);
        if (!originalAlbumFrames.isEmpty()) {
            pTrackMetadata->refAlbumInfo().setTitle(
                    firstNonEmptyFrameToQString(originalAlbumFrames));
        }
    }

    const TagLib::ID3v2::FrameList composerFrames(tag.frameListMap()["TCOM"]);
    if (!composerFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setComposer(
                firstNonEmptyFrameToQString(composerFrames));
    }

    // Apple decided to store the Work in the traditional ID3v2 Content Group
    // frame (TIT1) and introduced new Grouping (GRP1) and Movement Name (MVNM)
    // frames.
    // https://discussions.apple.com/thread/7900430
    // http://blog.jthink.net/2016/11/the-reason-why-is-grouping-field-no.html
    if (tag.frameListMap().contains("GRP1")) {
        // New grouping/work mapping
        const TagLib::ID3v2::FrameList appleGroupingFrames = tag.frameListMap()["GRP1"];
        if (!appleGroupingFrames.isEmpty()) {
            pTrackMetadata->refTrackInfo().setGrouping(
                    firstNonEmptyFrameToQString(appleGroupingFrames));
        }
#if defined(__EXTRA_METADATA__)
        const TagLib::ID3v2::FrameList workFrames = tag.frameListMap()["TIT1"];
        if (!workFrames.isEmpty()) {
            pTrackMetadata->refTrackInfo().setWork(
                    firstNonEmptyFrameToQString(workFrames));
        }
        const TagLib::ID3v2::FrameList movementFrames = tag.frameListMap()["MVNM"];
        if (!movementFrames.isEmpty()) {
            pTrackMetadata->refTrackInfo().setMovement(
                    firstNonEmptyFrameToQString(movementFrames));
        }
#endif // __EXTRA_METADATA__
    } else {
        // No Apple grouping frame found -> Use the traditional mapping
        const TagLib::ID3v2::FrameList traditionalGroupingFrames = tag.frameListMap()["TIT1"];
        if (!traditionalGroupingFrames.isEmpty()) {
            pTrackMetadata->refTrackInfo().setGrouping(
                    firstNonEmptyFrameToQString(traditionalGroupingFrames));
        }
    }

    // ID3v2.4.0: TDRC replaces TYER + TDAT
    const QString recordingTime(
            firstNonEmptyFrameToQString(tag.frameListMap()["TDRC"]));
    if ((tag.header()->majorVersion() >= 4) && !recordingTime.isEmpty()) {
        pTrackMetadata->refTrackInfo().setYear(recordingTime);
    } else {
        // Fallback to TYER + TDAT
        const QString recordingYear(
                firstNonEmptyFrameToQString(tag.frameListMap()["TYER"]).trimmed());
        QString year(recordingYear);
        if (kFormatTYER.length() == recordingYear.length()) {
            const QString recordingDate(
                    firstNonEmptyFrameToQString(tag.frameListMap()["TDAT"]).trimmed());
            if (kFormatTDAT.length() == recordingDate.length()) {
                const QDate date(
                        QDate::fromString(
                                recordingYear + recordingDate,
                                kFormatTYER + kFormatTDAT));
                if (date.isValid()) {
                    year = TrackMetadata::formatDate(date);
                }
            }
        }
        if (!year.isEmpty()) {
            pTrackMetadata->refTrackInfo().setYear(year);
        }
    }

    const TagLib::ID3v2::FrameList trackNumberFrames(tag.frameListMap()["TRCK"]);
    if (!trackNumberFrames.isEmpty()) {
        QString trackNumber;
        QString trackTotal;
        TrackNumbers::splitString(
                firstNonEmptyFrameToQString(trackNumberFrames),
                &trackNumber,
                &trackTotal);
        pTrackMetadata->refTrackInfo().setTrackNumber(trackNumber);
        pTrackMetadata->refTrackInfo().setTrackTotal(trackTotal);
    }

#if defined(__EXTRA_METADATA__)
    const TagLib::ID3v2::FrameList discNumberFrames(tag.frameListMap()["TPOS"]);
    if (!discNumberFrames.isEmpty()) {
        QString discNumber;
        QString discTotal;
        TrackNumbers::splitString(
                firstNonEmptyFrameToQString(discNumberFrames),
                &discNumber,
                &discTotal);
        pTrackMetadata->refTrackInfo().setDiscNumber(discNumber);
        pTrackMetadata->refTrackInfo().setDiscTotal(discTotal);
    }
#endif // __EXTRA_METADATA__

    const TagLib::ID3v2::FrameList bpmFrames(tag.frameListMap()["TBPM"]);
    if (!bpmFrames.isEmpty()) {
        parseBpm(pTrackMetadata,
                firstNonEmptyFrameToQString(bpmFrames));
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

    const TagLib::ID3v2::FrameList keyFrames(tag.frameListMap()["TKEY"]);
    if (!keyFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setKey(
                firstNonEmptyFrameToQString(keyFrames));
    }

    QString trackGain =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("REPLAYGAIN_TRACK_GAIN"));
    if (!trackGain.isEmpty()) {
        parseTrackGain(pTrackMetadata, trackGain);
    }
    QString trackPeak =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("REPLAYGAIN_TRACK_PEAK"));
    if (!trackPeak.isEmpty()) {
        parseTrackPeak(pTrackMetadata, trackPeak);
    }

#if defined(__EXTRA_METADATA__)
    QString albumGain =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("REPLAYGAIN_ALBUM_GAIN"));
    if (!albumGain.isEmpty()) {
        parseAlbumGain(pTrackMetadata, albumGain);
    }
    QString albumPeak =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("REPLAYGAIN_ALBUM_PEAK"));
    if (!albumPeak.isEmpty()) {
        parseAlbumPeak(pTrackMetadata, albumPeak);
    }

    QString trackArtistId =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("MusicBrainz Artist Id"));
    if (!trackArtistId.isNull()) {
        pTrackMetadata->refTrackInfo().setMusicBrainzArtistId(QUuid(trackArtistId));
    }
    QByteArray trackRecordingId =
            readFirstUniqueFileIdentifierFrame(
                    tag,
                    kMusicBrainzOwner);
    if (!trackRecordingId.isEmpty()) {
        pTrackMetadata->refTrackInfo().setMusicBrainzRecordingId(QUuid(trackRecordingId));
    }
    QString trackReleaseId =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("MusicBrainz Release Track Id"));
    if (!trackReleaseId.isNull()) {
        pTrackMetadata->refTrackInfo().setMusicBrainzReleaseId(QUuid(trackReleaseId));
    }
    QString trackWorkId =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("MusicBrainz Work Id"));
    if (!trackWorkId.isNull()) {
        pTrackMetadata->refTrackInfo().setMusicBrainzWorkId(QUuid(trackWorkId));
    }
    QString albumArtistId =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("MusicBrainz Album Artist Id"));
    if (!albumArtistId.isNull()) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzArtistId(QUuid(albumArtistId));
    }
    QString albumReleaseId =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("MusicBrainz Album Id"));
    if (!albumReleaseId.isNull()) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseId(QUuid(albumReleaseId));
    }
    QString albumReleaseGroupId =
            readFirstUserTextIdentificationFrame(
                    tag,
                    QStringLiteral("MusicBrainz Release Group Id"));
    if (!albumReleaseGroupId.isNull()) {
        pTrackMetadata->refAlbumInfo().setMusicBrainzReleaseGroupId(QUuid(albumReleaseGroupId));
    }

    const TagLib::ID3v2::FrameList conductorFrames(tag.frameListMap()["TPE3"]);
    if (!conductorFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setConductor(
                firstNonEmptyFrameToQString(conductorFrames));
    }
    const TagLib::ID3v2::FrameList isrcFrames(tag.frameListMap()["TSRC"]);
    if (!isrcFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setISRC(
                firstNonEmptyFrameToQString(isrcFrames));
    }
    const TagLib::ID3v2::FrameList languageFrames(tag.frameListMap()["TLAN"]);
    if (!languageFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setLanguage(
                firstNonEmptyFrameToQString(languageFrames));
    }
    const TagLib::ID3v2::FrameList lyricistFrames(tag.frameListMap()["TEXT"]);
    if (!lyricistFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setLyricist(
                firstNonEmptyFrameToQString(lyricistFrames));
    }
    if (tag.header()->majorVersion() >= 4) {
        const TagLib::ID3v2::FrameList moodFrames(tag.frameListMap()["TMOO"]);
        if (!moodFrames.isEmpty()) {
            pTrackMetadata->refTrackInfo().setMood(
                    firstNonEmptyFrameToQString(moodFrames));
        }
    }
    const TagLib::ID3v2::FrameList copyrightFrames(tag.frameListMap()["TCOP"]);
    if (!copyrightFrames.isEmpty()) {
        pTrackMetadata->refAlbumInfo().setCopyright(
                firstNonEmptyFrameToQString(copyrightFrames));
    }
    const TagLib::ID3v2::FrameList licenseFrames(tag.frameListMap()["WCOP"]);
    if (!licenseFrames.isEmpty()) {
        pTrackMetadata->refAlbumInfo().setLicense(
                firstNonEmptyFrameToQString(licenseFrames));
    }
    const TagLib::ID3v2::FrameList recordLabelFrames(tag.frameListMap()["TPUB"]);
    if (!recordLabelFrames.isEmpty()) {
        pTrackMetadata->refAlbumInfo().setRecordLabel(
                firstNonEmptyFrameToQString(recordLabelFrames));
    }
    const TagLib::ID3v2::FrameList remixerFrames(tag.frameListMap()["TPE4"]);
    if (!remixerFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setRemixer(
                firstNonEmptyFrameToQString(remixerFrames));
    }
    const TagLib::ID3v2::FrameList subtitleFrames(tag.frameListMap()["TIT3"]);
    if (!subtitleFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setSubtitle(
                firstNonEmptyFrameToQString(subtitleFrames));
    }
    const TagLib::ID3v2::FrameList encoderFrames(tag.frameListMap()["TENC"]);
    if (!encoderFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setEncoder(
                firstNonEmptyFrameToQString(encoderFrames));
    }
    const TagLib::ID3v2::FrameList encoderSettingsFrames(tag.frameListMap()["TSSE"]);
    if (!encoderSettingsFrames.isEmpty()) {
        pTrackMetadata->refTrackInfo().setEncoderSettings(
                firstNonEmptyFrameToQString(encoderSettingsFrames));
    }
#endif // __EXTRA_METADATA__

    // Serato tags
    const QByteArray seratoBeatGrid =
            readFirstGeneralEncapsulatedObjectFrame(
                    tag,
                    kFrameDescriptionSeratoBeatGrid);
    if (!seratoBeatGrid.isEmpty()) {
        parseSeratoBeatGrid(pTrackMetadata, seratoBeatGrid, FileType::MP3);
    }
    const QByteArray seratoMarkers =
            readFirstGeneralEncapsulatedObjectFrame(
                    tag,
                    kFrameDescriptionSeratoMarkers);
    if (!seratoMarkers.isEmpty()) {
        parseSeratoMarkers(pTrackMetadata, seratoMarkers, FileType::MP3);
    }
    const QByteArray seratoMarkers2 =
            readFirstGeneralEncapsulatedObjectFrame(
                    tag,
                    kFrameDescriptionSeratoMarkers2);
    if (!seratoMarkers2.isEmpty()) {
        parseSeratoMarkers2(pTrackMetadata, seratoMarkers2, FileType::MP3);
    }
}

bool exportTrackMetadataIntoTag(TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    const TagLib::ID3v2::Header* pHeader = pTag->header();
    DEBUG_ASSERT(pHeader);
    if (!checkHeaderVersionSupported(*pHeader)) {
        kLogger.warning() << "Legacy ID3v2 version - exporting only basic tags";
        taglib::exportTrackMetadataIntoTag(pTag, trackMetadata, WriteTagFlag::OmitNone);
        return true; // done
    }

    // NOTE(uklotzde): Setting the comment for ID3v2 tags does
    // not work as expected when using TagLib 1.9.1 and must
    // be skipped! Otherwise special purpose comment fields
    // with a description like "iTunSMPB" might be overwritten.
    // Mixxx implements a special case handling for ID3v2 comment
    // frames (see below)
    taglib::exportTrackMetadataIntoTag(
            pTag,
            trackMetadata,
            WriteTagFlag::OmitTrackNumber | WriteTagFlag::OmitYear | WriteTagFlag::OmitComment);

    // Writing the common comments frame has been omitted (see above)
    writeCommentsFrameWithoutDescription(
            pTag,
            toTString(trackMetadata.getTrackInfo().getComment()));

    writeTextIdentificationFrame(
            pTag,
            "TRCK",
            TrackNumbers::joinAsString(
                    trackMetadata.getTrackInfo().getTrackNumber(),
                    trackMetadata.getTrackInfo().getTrackTotal()));

    // NOTE(uklotz): Need to overwrite the TDRC frame if it
    // already exists. TagLib (1.9.x) writes a TDRC frame
    // even for ID3v2.3.0 tags if the numeric year is set.
    if ((pHeader->majorVersion() >= 4) || !pTag->frameList("TDRC").isEmpty()) {
        writeTextIdentificationFrame(
                pTag,
                "TDRC",
                trackMetadata.getTrackInfo().getYear());
    }
    if (pHeader->majorVersion() < 4) {
        // Fallback to TYER + TDAT
        const QDate date(TrackMetadata::parseDate(trackMetadata.getTrackInfo().getYear()));
        if (date.isValid()) {
            // Valid date
            writeTextIdentificationFrame(
                    pTag,
                    "TYER",
                    date.toString(kFormatTYER),
                    true);
            writeTextIdentificationFrame(
                    pTag,
                    "TDAT",
                    date.toString(kFormatTDAT),
                    true);
        } else {
            // Fallback to calendar year
            bool calendarYearValid = false;
            const QString calendarYear =
                    TrackMetadata::formatCalendarYear(
                            trackMetadata.getTrackInfo().getYear(),
                            &calendarYearValid);
            if (calendarYearValid) {
                writeTextIdentificationFrame(
                        pTag,
                        "TYER",
                        calendarYear,
                        true);
            }
        }
    }

    writeTextIdentificationFrame(
            pTag,
            "TPE2",
            trackMetadata.getAlbumInfo().getArtist());
    writeTextIdentificationFrame(
            pTag,
            "TCOM",
            trackMetadata.getTrackInfo().getComposer());

    // We can use the TIT1 frame only once, either for storing the Work
    // like Apple decided to do or traditionally for the Content Group.
    // Rationale: If the the file already has one or more GRP1 frames
    // or if the track has a Work field then store the Grouping in a
    // GRP1 frame instead of using TIT1.
    // See also: importTrackMetadataFromTag()
    if (
#if defined(__EXTRA_METADATA__)
            !trackMetadata.getTrackInfo().getWork().isNull() ||
            !trackMetadata.getTrackInfo().getMovement().isNull() ||
#endif // __EXTRA_METADATA__
            pTag->frameListMap().contains("GRP1")) {
        // New grouping/work/movement mapping if properties for classical
        // music are available or if the GRP1 frame is already present in
        // the file.
        writeTextIdentificationFrame(
                pTag,
                "GRP1",
                trackMetadata.getTrackInfo().getGrouping());
#if defined(__EXTRA_METADATA__)
        writeTextIdentificationFrame(
                pTag,
                "TIT1",
                trackMetadata.getTrackInfo().getWork());
        writeTextIdentificationFrame(
                pTag,
                "MVNM",
                trackMetadata.getTrackInfo().getMovement());
#endif // __EXTRA_METADATA__
    } else {
        // Stick to the traditional CONTENTGROUP mapping.
        writeTextIdentificationFrame(
                pTag,
                "TIT1",
                trackMetadata.getTrackInfo().getGrouping());
    }

    // According to the specification "The 'TBPM' frame contains the number
    // of beats per minute in the mainpart of the audio. The BPM is an
    // integer and represented as a numerical string."
    // Reference: http://id3.org/id3v2.3.0
    writeTextIdentificationFrame(
            pTag,
            "TBPM",
            formatBpmInteger(trackMetadata),
            true);

    writeTextIdentificationFrame(
            pTag,
            "TKEY",
            trackMetadata.getTrackInfo().getKey());

    writeUserTextIdentificationFrame(
            pTag,
            "REPLAYGAIN_TRACK_GAIN",
            formatTrackGain(trackMetadata),
            true);
    writeUserTextIdentificationFrame(
            pTag,
            "REPLAYGAIN_TRACK_PEAK",
            formatTrackPeak(trackMetadata),
            true);

#if defined(__EXTRA_METADATA__)
    writeTextIdentificationFrame(
            pTag,
            "TPOS",
            TrackNumbers::joinAsString(
                    trackMetadata.getTrackInfo().getDiscNumber(),
                    trackMetadata.getTrackInfo().getDiscTotal()));

    writeUserTextIdentificationFrame(
            pTag,
            "REPLAYGAIN_ALBUM_GAIN",
            formatAlbumGain(trackMetadata),
            true);
    writeUserTextIdentificationFrame(
            pTag,
            "REPLAYGAIN_ALBUM_PEAK",
            formatAlbumPeak(trackMetadata),
            true);

    writeUserTextIdentificationFrame(
            pTag,
            "MusicBrainz Artist Id",
            uuidToNullableStringWithoutBraces(
                    trackMetadata.getTrackInfo().getMusicBrainzArtistId()),
            false);
    {
        QByteArray identifier = trackMetadata.getTrackInfo().getMusicBrainzRecordingId().toByteArray();
        if (identifier.size() == 38) {
            // Strip leading/trailing curly braces
            DEBUG_ASSERT(identifier.startsWith('{'));
            DEBUG_ASSERT(identifier.endsWith('}'));
            identifier = identifier.mid(1, 36);
        }
        DEBUG_ASSERT(identifier.size() == 36);
        writeUniqueFileIdentifierFrame(
                pTag,
                kMusicBrainzOwner,
                identifier);
    }
    writeUserTextIdentificationFrame(
            pTag,
            "MusicBrainz Release Track Id",
            uuidToNullableStringWithoutBraces(trackMetadata.getTrackInfo().getMusicBrainzReleaseId()),
            false);
    writeUserTextIdentificationFrame(
            pTag,
            "MusicBrainz Work Id",
            uuidToNullableStringWithoutBraces(trackMetadata.getTrackInfo().getMusicBrainzWorkId()),
            false);
    writeUserTextIdentificationFrame(
            pTag,
            "MusicBrainz Album Artist Id",
            uuidToNullableStringWithoutBraces(trackMetadata.getAlbumInfo().getMusicBrainzArtistId()),
            false);
    writeUserTextIdentificationFrame(
            pTag,
            "MusicBrainz Album Id",
            uuidToNullableStringWithoutBraces(trackMetadata.getAlbumInfo().getMusicBrainzReleaseId()),
            false);
    writeUserTextIdentificationFrame(
            pTag,
            "MusicBrainz Release Group Id",
            uuidToNullableStringWithoutBraces(trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId()),
            false);

    writeTextIdentificationFrame(
            pTag,
            "TPE3",
            trackMetadata.getTrackInfo().getConductor());
    writeTextIdentificationFrame(
            pTag,
            "TSRC",
            trackMetadata.getTrackInfo().getISRC());
    writeTextIdentificationFrame(
            pTag,
            "TLAN",
            trackMetadata.getTrackInfo().getLanguage());
    writeTextIdentificationFrame(
            pTag,
            "TEXT",
            trackMetadata.getTrackInfo().getLyricist());
    if (pHeader->majorVersion() >= 4) {
        writeTextIdentificationFrame(
                pTag,
                "TMOO",
                trackMetadata.getTrackInfo().getMood());
    }
    writeTextIdentificationFrame(
            pTag,
            "TCOP",
            trackMetadata.getAlbumInfo().getCopyright());
    writeTextIdentificationFrame(
            pTag,
            "WCOP",
            trackMetadata.getAlbumInfo().getLicense());
    writeTextIdentificationFrame(
            pTag,
            "TPUB",
            trackMetadata.getAlbumInfo().getRecordLabel());
    writeTextIdentificationFrame(
            pTag,
            "TPE4",
            trackMetadata.getTrackInfo().getRemixer());
    writeTextIdentificationFrame(
            pTag,
            "TIT3",
            trackMetadata.getTrackInfo().getSubtitle());
    writeTextIdentificationFrame(
            pTag,
            "TENC",
            trackMetadata.getTrackInfo().getEncoder());
    writeTextIdentificationFrame(
            pTag,
            "TSSE",
            trackMetadata.getTrackInfo().getEncoderSettings());
#endif // __EXTRA_METADATA__

    // Export of Serato markers is disabled, because Mixxx
    // does not modify them.
#if defined(__EXPORT_SERATO_MARKERS__)
    // Serato tags
    writeGeneralEncapsulatedObjectFrame(
            pTag,
            kFrameDescriptionSeratoBeatGrid,
            trackMetadata.getTrackInfo().getSeratoTags().dumpBeatGrid(FileType::MP3));
    writeGeneralEncapsulatedObjectFrame(
            pTag,
            kFrameDescriptionSeratoMarkers,
            trackMetadata.getTrackInfo().getSeratoTags().dumpMarkers(FileType::MP3));
    writeGeneralEncapsulatedObjectFrame(
            pTag,
            kFrameDescriptionSeratoMarkers2,
            trackMetadata.getTrackInfo().getSeratoTags().dumpMarkers2(FileType::MP3));
#endif // __EXPORT_SERATO_MARKERS__

    return true;
}

} // namespace id3v2

} // namespace taglib

} // namespace mixxx
