#if defined(_MSC_VER)
#pragma warning(push)
// https://github.com/taglib/taglib/issues/1185
// warning C4251: 'TagLib::FileName::m_wname': class
// 'std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>'
// needs to have dll-interface to be used by clients of class 'TagLib::FileName'
#pragma warning(disable : 4251)
#endif

#include "track/taglib/trackmetadata_file.h"

#include <tfile.h>

#include "moc_trackmetadata_file.cpp"
#include "track/taglib/trackmetadata_common.h"
#include "util/logger.h"

namespace mixxx {

namespace {

Logger kLogger("TagLib");

void readAudioProperties(
        TrackMetadata* pTrackMetadata,
        const TagLib::AudioProperties& audioProperties) {
    DEBUG_ASSERT(pTrackMetadata);

    // NOTE(uklotzde): All audio properties will be updated
    // with the actual (and more precise) values when opening
    // the audio source for this track. Often those properties
    // stored in tags are imprecise and don't match the actual
    // audio data of the stream.
    pTrackMetadata->setStreamInfo(
            audio::StreamInfo{
                    audio::SignalInfo{
                            audio::ChannelCount(audioProperties.channels()),
                            audio::SampleRate(audioProperties.sampleRate()),
                    },
                    audio::Bitrate(audioProperties.bitrate()),
                    Duration::fromMillis(audioProperties.lengthInMilliseconds()),
            });
}

} // anonymous namespace

namespace taglib {

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
using namespace Qt::Literals::StringLiterals;
#else
constexpr inline QLatin1String operator""_L1(const char* str, size_t size) noexcept {
    return QLatin1String{str, static_cast<int>(size)};
}
#endif

// Deduce the TagLib file type from the file name
FileType stringToEnumFileType(
        const QString& fileType) {
    DEBUG_ASSERT(!fileType.isEmpty());

    struct TypePair {
        QLatin1String strType;
        FileType eType;
    };

    // This table is aligned with detectByExtension() in fileref.cpp
    constexpr static std::array lookupTable = {
            TypePair{"mp3"_L1, FileType::MPEG},
            TypePair{"mp2"_L1, FileType::MPEG},
            TypePair{"aac"_L1, FileType::MPEG},
            TypePair{"mpeg"_L1, FileType::MPEG},
            TypePair{"ogg"_L1, FileType::OggVorbis},
            TypePair{"oga"_L1, FileType::OggFlac},
            TypePair{"flac"_L1, FileType::FLAC},
            TypePair{"mpc"_L1, FileType::MPC},
            TypePair{"wv"_L1, FileType::WavPack},
            TypePair{"spx"_L1, FileType::Speex},
            TypePair{"opus"_L1, FileType::Opus},
            TypePair{"tta"_L1, FileType::TrueAudio},
            TypePair{"caf"_L1, FileType::MP4},
            TypePair{"m4a"_L1, FileType::MP4},
            TypePair{"stem.m4a"_L1, FileType::MP4},
            TypePair{"m4r"_L1, FileType::MP4},
            TypePair{"m4b"_L1, FileType::MP4},
            TypePair{"m4p"_L1, FileType::MP4},
            TypePair{"m4v"_L1, FileType::MP4},
            TypePair{"mj2"_L1, FileType::MP4},
            TypePair{"mov"_L1, FileType::MP4},
            TypePair{"mp4"_L1, FileType::MP4},
            TypePair{"stem.mp4"_L1, FileType::MP4},
            TypePair{"3gp"_L1, FileType::MP4},
            TypePair{"3g2"_L1, FileType::MP4},
            TypePair{"wma"_L1, FileType::ASF},
            TypePair{"asf"_L1, FileType::ASF},
            TypePair{"aiff"_L1, FileType::AIFF},
            TypePair{"aifc"_L1, FileType::AIFF},
            TypePair{"wav"_L1, FileType::WAV},
            TypePair{"ape"_L1, FileType::APE},
            TypePair{"it"_L1, FileType::IT},
            TypePair{"mod"_L1, FileType::Mod},
            TypePair{"module"_L1, FileType::Mod},
            TypePair{"nst"_L1, FileType::Mod},
            TypePair{"wow"_L1, FileType::Mod},
            TypePair{"s3m"_L1, FileType::S3M},
            TypePair{"xm"_L1, FileType::XM},
            TypePair{"dsf"_L1, FileType::DSF},
            TypePair{"dff"_L1, FileType::DSDIFF},
            TypePair{"dsdiff"_L1, FileType::DSDIFF}};

    // NOLINTNEXTLINE(readability-qualified-auto)
    const auto it = std::find_if(
            lookupTable.cbegin(),
            lookupTable.cend(),
            [fileType](const auto& pair) { return pair.strType == fileType; });
    return it != lookupTable.end() ? it->eType : FileType::Unknown;
}

QDebug operator<<(QDebug debug, FileType fileType) {
    return debug << static_cast<std::underlying_type<FileType>::type>(fileType);
}

bool hasID3v1Tag(TagLib::MPEG::File& file) {
    return file.hasID3v1Tag();
}

bool hasID3v2Tag(TagLib::MPEG::File& file) {
    return file.hasID3v2Tag();
}

bool hasAPETag(TagLib::MPEG::File& file) {
    return file.hasAPETag();
}

bool hasID3v2Tag(TagLib::FLAC::File& file) {
    return file.hasID3v2Tag();
}

bool hasXiphComment(TagLib::FLAC::File& file) {
    return file.hasXiphComment();
}

bool hasAPETag(TagLib::WavPack::File& file) {
    return file.hasAPETag();
}

bool hasID3v2Tag(TagLib::RIFF::WAV::File& file) {
    return file.hasID3v2Tag();
}

bool hasID3v2Tag(TagLib::RIFF::AIFF::File& file) {
    return file.hasID3v2Tag();
}

bool hasMP4Tag(TagLib::MP4::File& file) {
    // Note (TagLib 1.11.1): For MP4 files without file tags
    // TagLib still reports that the MP4 tag exists. Additionally
    // we need to check that the tag itself is not empty.
    return file.hasMP4Tag() && !file.tag()->isEmpty();
}

bool readAudioPropertiesFromFile(
        TrackMetadata* pTrackMetadata,
        const TagLib::File& file) {
    // The declaration of TagLib::FileName is platform specific.
#ifdef _WIN32
    // For _WIN32 there are two types std::string and std::wstring
    // we must pick one explicit,
    // to prevent "use of overloaded operator '<<' is ambiguous" error
    // on clang-cl builds.
    // We need to save the filename here because if it was chained
    // (`file.name().wstr()`) the wstr() result would be dangling.
    TagLib::FileName filename_owning = file.name();
    const std::wstring& filename = filename_owning.wstr();
#else
    const char* filename = file.name();
#endif
    if (!file.isValid()) {
        kLogger.warning() << "Cannot read audio properties from "
                             "inaccessible/unreadable/invalid file:"
                          << filename;
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
                          << filename;
        return false;
    }
    readAudioProperties(pTrackMetadata, *pAudioProperties);
    return true;
}

} // namespace taglib

} // namespace mixxx

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
