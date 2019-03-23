#pragma once

#include <taglib/apetag.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>
#include <taglib/infotag.h>

#include <taglib/flacfile.h>
#include <taglib/mpegfile.h>
#include <taglib/wavpackfile.h>

// TagLib has support for the Ogg Opus file format since version 1.9
#define TAGLIB_HAS_OPUSFILE \
    ((TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9)))

// TagLib has support for hasID3v2Tag()/ID3v2Tag() for WAV files since version 1.9
#define TAGLIB_HAS_WAV_ID3V2TAG \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))

// TagLib has support for hasID3v2Tag() for AIFF files since version 1.10
#define TAGLIB_HAS_AIFF_HAS_ID3V2TAG \
    (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 10))

#include <QImage>
#include <QFile>

#include "track/trackmetadata.h"


namespace mixxx {

namespace taglib {

enum class FileType {
    UNKNOWN,
    AIFF,
    FLAC,
    MP3,
    MP4,
    OGG,
    OPUS,
    WAV,
    WV
};

QDebug operator<<(QDebug debug, FileType fileType);

// Deduce the TagLib file type from the file name
FileType getFileTypeFromFileName(QString fileName);

bool readAudioProperties(
        TrackMetadata* pTrackMetadata,
        const TagLib::File& file);

void importCoverImageFromID3v2Tag(QImage* pCoverArt, const TagLib::ID3v2::Tag& tag);
void importCoverImageFromAPETag(QImage* pCoverArt, const TagLib::APE::Tag& tag);
void importCoverImageFromVorbisCommentTag(QImage* pCoverArt, TagLib::Ogg::XiphComment& tag);
void importCoverImageFromMP4Tag(QImage* pCoverArt, const TagLib::MP4::Tag& tag);

QImage importCoverImageFromVorbisCommentPictureList(const TagLib::List<TagLib::FLAC::Picture*>& pictures);

// Low-level tag read/write functions are exposed only for testing purposes!

// Bitmask of optional tag fields that should NOT be read from the
// common part of the tag through TagLib::Tag.
// Usage: The write functions for ID3v2, MP4, APE and XiphComment tags
// have specialized code for some or all of the corresponding tag fields
// and the common implementation sometime doesn't work as expected.
enum ReadTagMask {
    READ_TAG_OMIT_NONE         = 0x00,
    READ_TAG_OMIT_COMMENT      = 0x01,
};

void importTrackMetadataFromTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::Tag& tag,
        int readMask = READ_TAG_OMIT_NONE);
void importTrackMetadataFromID3v2Tag(
        TrackMetadata* pTrackMetadata,
        const TagLib::ID3v2::Tag& tag);
void importTrackMetadataFromAPETag(
        TrackMetadata* pTrackMetadata,
        const TagLib::APE::Tag& tag);
void importTrackMetadataFromVorbisCommentTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag);
void importTrackMetadataFromMP4Tag(
        TrackMetadata* pTrackMetadata,
        const TagLib::MP4::Tag& tag);
void importTrackMetadataFromRIFFTag(
        TrackMetadata* pTrackMetadata,
        const TagLib::RIFF::Info::Tag& tag);

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

void exportTrackMetadataIntoTag(
        TagLib::Tag* pTag,
        const TrackMetadata& trackMetadata,
        int writeMask);
bool exportTrackMetadataIntoID3v2Tag(
        TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata);
bool exportTrackMetadataIntoAPETag(
        TagLib::APE::Tag* pTag,
        const TrackMetadata& trackMetadata);
bool exportTrackMetadataIntoXiphComment(
        TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata);
bool exportTrackMetadataIntoMP4Tag(
        TagLib::MP4::Tag* pTag,
        const TrackMetadata& trackMetadata);

#ifdef _WIN32
static_assert(sizeof(wchar_t) == sizeof(QChar), "wchar_t is not the same size than QChar");
#define TAGLIB_FILENAME_FROM_QSTRING(fileName) (const wchar_t*)fileName.utf16()
// Note: we cannot use QString::toStdWString since QT 4 is compiled with
// '/Zc:wchar_t-' flag and QT 5 not
#else
#define TAGLIB_FILENAME_FROM_QSTRING(fileName) QFile::encodeName(fileName).constData()
#endif // _WIN32

// Some helper functions for backwards compatibility with older
// TagLib version that don't provide the corresponding member
// functions for files.
bool hasID3v1Tag(TagLib::MPEG::File& file);
bool hasID3v2Tag(TagLib::MPEG::File& file);
bool hasAPETag(TagLib::MPEG::File& file);
bool hasID3v2Tag(TagLib::FLAC::File& file);
bool hasXiphComment(TagLib::FLAC::File& file);
bool hasAPETag(TagLib::WavPack::File& file);

} // namespace taglib

} // namespace mixxx
