#pragma once

#include <aifffile.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mpegfile.h>
#include <wavfile.h>
#include <wavpackfile.h>

#include <QFile>

namespace mixxx {

// Forward declaration
class TrackMetadata;

namespace taglib {
Q_NAMESPACE

// This enum is aligned with TagLib_File_Type form the TagLib C binding
enum class FileType {
    Unknown = -1,
    MPEG = 0,
    OggVorbis,
    FLAC,
    MPC,
    OggFlac,
    WavPack,
    Speex,
    TrueAudio,
    MP4,
    ASF,
    AIFF,
    WAV,
    APE,
    IT,
    Mod,
    S3M,
    XM,
    Opus,
    DSF,
    DSDIFF
};
Q_ENUM_NS(FileType);

QDebug operator<<(QDebug debug, FileType fileType);

// Deduce enum FileType from the fileType String
FileType stringToEnumFileType(const QString& fileType);

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
