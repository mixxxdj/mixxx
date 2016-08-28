#ifndef TRACKMETADATATAGLIB_H
#define TRACKMETADATATAGLIB_H

#include <taglib/apetag.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/mp4tag.h>

#include <taglib/flacfile.h>
#include <taglib/mpegfile.h>
#include <taglib/wavpackfile.h>

#include <QImage>

#include "track/trackmetadata.h"
#include "util/result.h"

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

// Read both track metadata and cover art of supported file types.
// Both parameters are optional and might be NULL.
Result readTrackMetadataAndCoverArtFromFile(TrackMetadata* pTrackMetadata, QImage* pCoverArt, QString fileName, FileType fileType = FileType::UNKNOWN);

// Write track metadata into the file with the given name
Result writeTrackMetadataIntoFile(const TrackMetadata& trackMetadata, QString fileName, FileType fileType = FileType::UNKNOWN);

// Low-level tag read/write functions are exposed only for testing purposes!
void readTrackMetadataFromID3v2Tag(TrackMetadata* pTrackMetadata, const TagLib::ID3v2::Tag& tag);
void readTrackMetadataFromAPETag(TrackMetadata* pTrackMetadata, const TagLib::APE::Tag& tag);
void readTrackMetadataFromXiphComment(TrackMetadata* pTrackMetadata,
        const TagLib::Ogg::XiphComment& tag);
void readTrackMetadataFromMP4Tag(TrackMetadata* pTrackMetadata, const TagLib::MP4::Tag& tag);
bool writeTrackMetadataIntoID3v2Tag(TagLib::ID3v2::Tag* pTag,
        const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoAPETag(TagLib::APE::Tag* pTag, const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoXiphComment(TagLib::Ogg::XiphComment* pTag,
        const TrackMetadata& trackMetadata);
bool writeTrackMetadataIntoMP4Tag(TagLib::MP4::Tag* pTag, const TrackMetadata& trackMetadata);

#ifdef _WIN32
static_assert(sizeof(wchar_t) == sizeof(QChar), "wchar_t is not the same size than QChar");
#define TAGLIB_FILENAME_FROM_QSTRING(fileName) (const wchar_t*)fileName.utf16()
// Note: we cannot use QString::toStdWString since QT 4 is compiled with
// '/Zc:wchar_t-' flag and QT 5 not
#else
#define TAGLIB_FILENAME_FROM_QSTRING(fileName) (fileName).toLocal8Bit().constData()
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

#endif
