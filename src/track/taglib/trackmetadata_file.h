#pragma once

#include <taglib/aifffile.h>
#include <taglib/flacfile.h>
#include <taglib/mp4file.h>
#include <taglib/mpegfile.h>
#include <taglib/wavfile.h>
#include <taglib/wavpackfile.h>

#include <QFile>

namespace mixxx {

// Forward declaration
class TrackMetadata;

namespace taglib {

enum class FileType {
    Unknown,
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
FileType getFileTypeFromFileName(const QString& fileName);

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
bool hasAPETag(TagLib::MPEG::File& file);
bool hasAPETag(TagLib::WavPack::File& file);
bool hasID3v1Tag(TagLib::MPEG::File& file);
bool hasID3v2Tag(TagLib::MPEG::File& file);
bool hasID3v2Tag(TagLib::FLAC::File& file);
bool hasID3v2Tag(TagLib::RIFF::WAV::File& file);
bool hasID3v2Tag(TagLib::RIFF::AIFF::File& file);
bool hasMP4Tag(TagLib::MP4::File& file);
bool hasXiphComment(TagLib::FLAC::File& file);

bool readAudioPropertiesFromFile(
        TrackMetadata* pTrackMetadata,
        const TagLib::File& file);

} // namespace taglib

} // namespace mixxx
