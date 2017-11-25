#include "sources/metadatasourcetaglib.h"

#include "track/trackmetadatataglib.h"

#include "util/logger.h"
#include "util/memory.h"

#include <QFileInfo>

#include <taglib/mp4file.h>
#include <taglib/vorbisfile.h>
#if (TAGLIB_HAS_OPUSFILE)
#include <taglib/opusfile.h>
#endif
#include <taglib/wavfile.h>
#include <taglib/aifffile.h>


namespace mixxx {

namespace {

Logger kLogger("MetadataSourceTagLib");

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

    void importTrackMetadataFromTextChunks(TrackMetadata* pTrackMetadata) /*non-const*/ {
        if (pTrackMetadata == nullptr) {
            return; // nothing to do
        }
        for(unsigned int i = 0; i < chunkCount(); ++i) {
            const TagLib::ByteVector chunkId(TagLib::RIFF::AIFF::File::chunkName(i));
            if (chunkId == "NAME") {
                pTrackMetadata->refTrackInfo().setTitle(decodeChunkText(
                        TagLib::RIFF::AIFF::File::chunkData(i)));
            } else if (chunkId == "AUTH") {
                pTrackMetadata->refTrackInfo().setArtist(decodeChunkText(
                        TagLib::RIFF::AIFF::File::chunkData(i)));
            } else if (chunkId == "ANNO") {
                pTrackMetadata->refTrackInfo().setComment(decodeChunkText(
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


std::pair<MetadataSourceTagLib::ImportResult, QDateTime>
MetadataSourceTagLib::afterImportSucceeded() const {
    return std::make_pair(ImportResult::Succeeded, QFileInfo(m_fileName).lastModified());
}

std::pair<MetadataSourceTagLib::ExportResult, QDateTime>
MetadataSourceTagLib::afterExportSucceeded() const {
    return std::make_pair(ExportResult::Succeeded, QFileInfo(m_fileName).lastModified());
}

std::pair<MetadataSource::ImportResult, QDateTime>
MetadataSourceTagLib::importTrackMetadataAndCoverImage(
        TrackMetadata* pTrackMetadata,
        QImage* pCoverImage) const {
    VERIFY_OR_DEBUG_ASSERT(pTrackMetadata || pCoverImage) {
        kLogger.warning()
                << "Nothing to import"
                << "from file" << m_fileName
                << "with type" << m_fileType;
        return std::make_pair(MetadataSource::ImportResult::Unavailable, QDateTime());
    }
    kLogger.trace() << "Importing"
            << ((pTrackMetadata && pCoverImage) ? "track metadata and cover art" : (pTrackMetadata ? "track metadata" : "cover art"))
            << "from file" << m_fileName
            << "with type" << m_fileType;

    // Rationale: If a file contains different types of tags only
    // a single type of tag will be read. Tag types are read in a
    // fixed order. Both track metadata and cover art will be read
    // from the same tag types. Only the first available tag type
    // is read and data in subsequent tags is ignored.

    switch (m_fileType) {
    case taglib::FileType::MP3:
    {
        TagLib::MPEG::File file(TAGLIB_FILENAME_FROM_QSTRING(m_fileName));
        if (taglib::readAudioProperties(pTrackMetadata, file)) {
            const TagLib::ID3v2::Tag* pID3v2Tag =
                    taglib::hasID3v2Tag(file) ? file.ID3v2Tag() : nullptr;
            if (pID3v2Tag) {
                taglib::importTrackMetadataFromID3v2Tag(pTrackMetadata, *pID3v2Tag);
                taglib::importCoverImageFromID3v2Tag(pCoverImage, *pID3v2Tag);
                return afterImportSucceeded();
            } else {
                const TagLib::APE::Tag* pAPETag =
                        taglib::hasAPETag(file) ? file.APETag() : nullptr;
                if (pAPETag) {
                    taglib::importTrackMetadataFromAPETag(pTrackMetadata, *pAPETag);
                    taglib::importCoverImageFromAPETag(pCoverImage, *pAPETag);
                    return afterImportSucceeded();
                } else {
                    // fallback
                    const TagLib::Tag* pTag(file.tag());
                    if (pTag) {
                        taglib::importTrackMetadataFromTag(pTrackMetadata, *pTag);
                        return afterImportSucceeded();
                    }
                }
            }
        }
        break;
    }
    case taglib::FileType::MP4:
    {
        TagLib::MP4::File file(TAGLIB_FILENAME_FROM_QSTRING(m_fileName));
        if (taglib::readAudioProperties(pTrackMetadata, file)) {
            const TagLib::MP4::Tag* pMP4Tag = file.tag();
            if (pMP4Tag) {
                taglib::importTrackMetadataFromMP4Tag(pTrackMetadata, *pMP4Tag);
                taglib::importCoverImageFromMP4Tag(pCoverImage, *pMP4Tag);
                return afterImportSucceeded();
            } else {
                // fallback
                const TagLib::Tag* pTag(file.tag());
                if (pTag) {
                    taglib::importTrackMetadataFromTag(pTrackMetadata, *pTag);
                    return afterImportSucceeded();
                }
            }
        }
        break;
    }
    case taglib::FileType::FLAC:
    {
        TagLib::FLAC::File file(TAGLIB_FILENAME_FROM_QSTRING(m_fileName));
        // Read cover art directly from the file first. Will be
        // overwritten with cover art contained in on of the tags.
        if (pCoverImage != nullptr) {
            *pCoverImage = taglib::importCoverImageFromVorbisCommentPictureList(file.pictureList());
        }
        if (taglib::readAudioProperties(pTrackMetadata, file)) {
            // VorbisComment tag takes precedence over ID3v2 tag
            TagLib::Ogg::XiphComment* pXiphComment =
                    taglib::hasXiphComment(file) ? file.xiphComment() : nullptr;
            if (pXiphComment) {
                taglib::importTrackMetadataFromVorbisCommentTag(pTrackMetadata, *pXiphComment);
                taglib::importCoverImageFromVorbisCommentTag(pCoverImage, *pXiphComment);
                return afterImportSucceeded();
            } else {
                const TagLib::ID3v2::Tag* pID3v2Tag =
                        taglib::hasID3v2Tag(file) ? file.ID3v2Tag() : nullptr;
                if (pID3v2Tag) {
                    taglib::importTrackMetadataFromID3v2Tag(pTrackMetadata, *pID3v2Tag);
                    taglib::importCoverImageFromID3v2Tag(pCoverImage, *pID3v2Tag);
                    return afterImportSucceeded();
                } else {
                    // fallback
                    const TagLib::Tag* pTag(file.tag());
                    if (pTag) {
                        taglib::importTrackMetadataFromTag(pTrackMetadata, *pTag);
                        return afterImportSucceeded();
                    }
                }
            }
        }
        break;
    }
    case taglib::FileType::OGG:
    {
        TagLib::Ogg::Vorbis::File file(TAGLIB_FILENAME_FROM_QSTRING(m_fileName));
        if (taglib::readAudioProperties(pTrackMetadata, file)) {
            TagLib::Ogg::XiphComment* pXiphComment = file.tag();
            if (pXiphComment) {
                taglib::importTrackMetadataFromVorbisCommentTag(pTrackMetadata, *pXiphComment);
                taglib::importCoverImageFromVorbisCommentTag(pCoverImage, *pXiphComment);
                return afterImportSucceeded();
            } else {
                // fallback
                const TagLib::Tag* pTag(file.tag());
                if (pTag) {
                    taglib::importTrackMetadataFromTag(pTrackMetadata, *pTag);
                    return afterImportSucceeded();
                }
            }
        }
        break;
    }
#if (TAGLIB_HAS_OPUSFILE)
    case taglib::FileType::OPUS:
    {
        TagLib::Ogg::Opus::File file(TAGLIB_FILENAME_FROM_QSTRING(m_fileName));
        if (taglib::readAudioProperties(pTrackMetadata, file)) {
            TagLib::Ogg::XiphComment* pXiphComment = file.tag();
            if (pXiphComment) {
                taglib::importTrackMetadataFromVorbisCommentTag(pTrackMetadata, *pXiphComment);
                taglib::importCoverImageFromVorbisCommentTag(pCoverImage, *pXiphComment);
                 return afterImportSucceeded();
            } else {
                // fallback
                const TagLib::Tag* pTag(file.tag());
                if (pTag) {
                    taglib::importTrackMetadataFromTag(pTrackMetadata, *pTag);
                    return afterImportSucceeded();
                }
            }
        }
        break;
    }
#endif // TAGLIB_HAS_OPUSFILE
    case taglib::FileType::WV:
    {
        TagLib::WavPack::File file(TAGLIB_FILENAME_FROM_QSTRING(m_fileName));
        if (taglib::readAudioProperties(pTrackMetadata, file)) {
            const TagLib::APE::Tag* pAPETag =
                    taglib::hasAPETag(file) ? file.APETag() : nullptr;
            if (pAPETag) {
                taglib::importTrackMetadataFromAPETag(pTrackMetadata, *pAPETag);
                taglib::importCoverImageFromAPETag(pCoverImage, *pAPETag);
                return afterImportSucceeded();
            } else {
                // fallback
                const TagLib::Tag* pTag(file.tag());
                if (pTag) {
                    taglib::importTrackMetadataFromTag(pTrackMetadata, *pTag);
                    return afterImportSucceeded();
                }
            }
        }
        break;
    }
    case taglib::FileType::WAV:
    {
        TagLib::RIFF::WAV::File file(TAGLIB_FILENAME_FROM_QSTRING(m_fileName));
        if (taglib::readAudioProperties(pTrackMetadata, file)) {
#if (TAGLIB_HAS_WAV_ID3V2TAG)
            const TagLib::ID3v2::Tag* pID3v2Tag =
                    file.hasID3v2Tag() ? file.ID3v2Tag() : nullptr;
#else
            const TagLib::ID3v2::Tag* pID3v2Tag = file.tag();
#endif
            if (pID3v2Tag) {
                taglib::importTrackMetadataFromID3v2Tag(pTrackMetadata, *pID3v2Tag);
                taglib::importCoverImageFromID3v2Tag(pCoverImage, *pID3v2Tag);
                return afterImportSucceeded();
            } else {
                // fallback
                const TagLib::RIFF::Info::Tag* pTag = file.InfoTag();
                if (pTag) {
                    taglib::importTrackMetadataFromRIFFTag(pTrackMetadata, *pTag);
                    return afterImportSucceeded();
                }
            }
        }
        break;
    }
    case taglib::FileType::AIFF:
    {
        AiffFile file(TAGLIB_FILENAME_FROM_QSTRING(m_fileName));
        if (taglib::readAudioProperties(pTrackMetadata, file)) {
#if (TAGLIB_HAS_AIFF_HAS_ID3V2TAG)
            const TagLib::ID3v2::Tag* pID3v2Tag = file.hasID3v2Tag() ? file.tag() : nullptr;
#else
            const TagLib::ID3v2::Tag* pID3v2Tag = file.tag();
#endif
            if (pID3v2Tag) {
                taglib::importTrackMetadataFromID3v2Tag(pTrackMetadata, *pID3v2Tag);
                taglib::importCoverImageFromID3v2Tag(pCoverImage, *pID3v2Tag);
            } else {
                // fallback
                file.importTrackMetadataFromTextChunks(pTrackMetadata);
            }
            return afterImportSucceeded();
        }
        break;
    }
    default:
        kLogger.warning()
            << "Cannot import track metadata"
            << "from file" << m_fileName
            << "with unknown or unsupported type" << m_fileType;
        return MetadataSource::importTrackMetadataAndCoverImage(pTrackMetadata, pCoverImage);
    }

    kLogger.warning()
            << "No track metadata or cover art found"
            << "in file" << m_fileName
            << "with type" << m_fileType;
    return MetadataSource::importTrackMetadataAndCoverImage(pTrackMetadata, pCoverImage);
}

namespace {

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
          m_modifiedTagsBitmask(exportTrackMetadata(&m_file, trackMetadata)) {
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
    static int exportTrackMetadata(TagLib::MPEG::File* pFile, const TrackMetadata& trackMetadata) {
        int modifiedTagsBitmask = TagLib::MPEG::File::NoTags;
        if (pFile->isOpen()) {
            TagLib::ID3v2::Tag* pID3v2Tag = nullptr;
            if (taglib::hasAPETag(*pFile)) {
                if (taglib::exportTrackMetadataIntoAPETag(pFile->APETag(), trackMetadata)) {
                    modifiedTagsBitmask |= TagLib::MPEG::File::APE;
                }
                // Only write ID3v2 tag if it already exists
                pID3v2Tag = pFile->ID3v2Tag(false);
            } else {
                // Get or create  ID3v2 tag
                pID3v2Tag = pFile->ID3v2Tag(true);
            }
            if (taglib::exportTrackMetadataIntoID3v2Tag(pID3v2Tag, trackMetadata)) {
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
          m_modifiedTags(exportTrackMetadata(&m_file, trackMetadata)) {
    }
    ~Mp4TagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool exportTrackMetadata(TagLib::MP4::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && taglib::exportTrackMetadataIntoMP4Tag(pFile->tag(), trackMetadata);
    }

    TagLib::MP4::File m_file;
    bool m_modifiedTags;
};

class FlacTagSaver: public TagSaver {
public:
    FlacTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(exportTrackMetadata(&m_file, trackMetadata)) {
    }
    ~FlacTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool exportTrackMetadata(TagLib::FLAC::File* pFile, const TrackMetadata& trackMetadata) {
        bool modifiedTags = false;
        if (pFile->isOpen()) {
            TagLib::Ogg::XiphComment* pXiphComment = nullptr;
            if (taglib::hasID3v2Tag(*pFile)) {
                modifiedTags |= taglib::exportTrackMetadataIntoID3v2Tag(pFile->ID3v2Tag(), trackMetadata);
                // Only write VorbisComment tag if it already exists
                pXiphComment = pFile->xiphComment(false);
            } else {
                // Get or create VorbisComment tag
                pXiphComment = pFile->xiphComment(true);
            }
            modifiedTags |= taglib::exportTrackMetadataIntoXiphComment(pXiphComment, trackMetadata);
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
          m_modifiedTags(exportTrackMetadata(&m_file, trackMetadata)) {
    }
    ~OggTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool exportTrackMetadata(TagLib::Ogg::Vorbis::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && taglib::exportTrackMetadataIntoXiphComment(pFile->tag(), trackMetadata);
    }

    TagLib::Ogg::Vorbis::File m_file;
    bool m_modifiedTags;
};

#if (TAGLIB_HAS_OPUSFILE)
class OpusTagSaver: public TagSaver {
public:
    OpusTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(exportTrackMetadata(&m_file, trackMetadata)) {
    }
    ~OpusTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool exportTrackMetadata(TagLib::Ogg::Opus::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && taglib::exportTrackMetadataIntoXiphComment(pFile->tag(), trackMetadata);
    }

    TagLib::Ogg::Opus::File m_file;
    bool m_modifiedTags;
};
#endif // TAGLIB_HAS_OPUSFILE

class WavPackTagSaver: public TagSaver {
public:
    WavPackTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(exportTrackMetadata(&m_file, trackMetadata)) {
    }
    ~WavPackTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool exportTrackMetadata(TagLib::WavPack::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && taglib::exportTrackMetadataIntoAPETag(pFile->APETag(true), trackMetadata);
    }

    TagLib::WavPack::File m_file;
    bool m_modifiedTags;
};

bool exportTrackMetadataIntoRIFFTag(TagLib::RIFF::Info::Tag* pTag, const TrackMetadata& trackMetadata) {
    if (!pTag) {
        return false;
    }

    taglib::exportTrackMetadataIntoTag(pTag, trackMetadata, taglib::WRITE_TAG_OMIT_NONE);

    return true;
}

class WavTagSaver: public TagSaver {
public:
    WavTagSaver(const QString& fileName, const TrackMetadata& trackMetadata)
        : m_file(TAGLIB_FILENAME_FROM_QSTRING(fileName)),
          m_modifiedTags(exportTrackMetadata(&m_file, trackMetadata)) {
    }
    ~WavTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool exportTrackMetadata(TagLib::RIFF::WAV::File* pFile, const TrackMetadata& trackMetadata) {
        bool modifiedTags = false;
        if (pFile->isOpen()) {
            // Write into all available tags
#if (TAGLIB_HAS_WAV_ID3V2TAG)
            modifiedTags |= taglib::exportTrackMetadataIntoID3v2Tag(pFile->ID3v2Tag(), trackMetadata);
#else
            modifiedTags |= taglib::exportTrackMetadataIntoID3v2Tag(pFile->tag(), trackMetadata);
#endif
            modifiedTags |= exportTrackMetadataIntoRIFFTag(pFile->InfoTag(), trackMetadata);
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
          m_modifiedTags(exportTrackMetadata(&m_file, trackMetadata)) {
    }
    ~AiffTagSaver() override {}

    bool hasModifiedTags() const override {
        return m_modifiedTags;
    }

    bool saveModifiedTags() override {
        return m_file.save();
    }

private:
    static bool exportTrackMetadata(TagLib::RIFF::AIFF::File* pFile, const TrackMetadata& trackMetadata) {
        return pFile->isOpen()
                && taglib::exportTrackMetadataIntoID3v2Tag(pFile->tag(), trackMetadata);
    }

    TagLib::RIFF::AIFF::File m_file;
    bool m_modifiedTags;
};

} // anonymous namespace

std::pair<MetadataSource::ExportResult, QDateTime>
MetadataSourceTagLib::exportTrackMetadata(
        const TrackMetadata& trackMetadata) const {
    kLogger.debug() << "Exporting track metadata"
            << "into file" << m_fileName
            << "with type" << m_fileType;

    std::unique_ptr<TagSaver> pTagSaver;
    switch (m_fileType) {
    case taglib::FileType::MP3:
    {
        pTagSaver = std::make_unique<MpegTagSaver>(m_fileName, trackMetadata);
        break;
    }
    case taglib::FileType::MP4:
    {
        pTagSaver = std::make_unique<Mp4TagSaver>(m_fileName, trackMetadata);
        break;
    }
    case taglib::FileType::FLAC:
    {
        pTagSaver = std::make_unique<FlacTagSaver>(m_fileName, trackMetadata);
        break;
    }
    case taglib::FileType::OGG:
    {
        pTagSaver = std::make_unique<OggTagSaver>(m_fileName, trackMetadata);
        break;
    }
#if (TAGLIB_HAS_OPUSFILE)
    case taglib::FileType::OPUS:
    {
        pTagSaver = std::make_unique<OpusTagSaver>(m_fileName, trackMetadata);
        break;
    }
#endif // TAGLIB_HAS_OPUSFILE
    case taglib::FileType::WV:
    {
        pTagSaver = std::make_unique<WavPackTagSaver>(m_fileName, trackMetadata);
        break;
    }
    case taglib::FileType::WAV:
    {
        pTagSaver = std::make_unique<WavTagSaver>(m_fileName, trackMetadata);
        break;
    }
    case taglib::FileType::AIFF:
    {
        pTagSaver = std::make_unique<AiffTagSaver>(m_fileName, trackMetadata);
        break;
    }
    default:
        kLogger.warning()
            << "Cannot export track metadata"
            << "into file" << m_fileName
            << "with unknown or unsupported type"
            << m_fileType;
        return MetadataSource::exportTrackMetadata(trackMetadata);
    }

    if (pTagSaver->hasModifiedTags()) {
        if (pTagSaver->saveModifiedTags()) {
            return afterExportSucceeded();
        } else {
            kLogger.warning() << "Failed to save tags of file" << m_fileName;
            return std::make_pair(ExportResult::Failed, QDateTime());
        }
    } else {
        kLogger.warning() << "Failed to modify tags of file" << m_fileName;
        return std::make_pair(ExportResult::Failed, QDateTime());
    }
}

} // namespace mixxx
