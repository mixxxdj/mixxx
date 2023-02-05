#include "track/taglib/trackmetadata_file.h"

#include <taglib/tfile.h>

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

// Deduce the TagLib file type from the file name
FileType getFileTypeFromFileName(
        const QString& fileName) {
    DEBUG_ASSERT(!fileName.isEmpty());
    const QString fileSuffix(fileName.section(QChar('.'), -1).toLower().trimmed());
    if (QStringLiteral("mp3") == fileSuffix) {
        return FileType::MP3;
    }
    if ((QStringLiteral("m4a") == fileSuffix) || (QStringLiteral("m4v") == fileSuffix)) {
        return FileType::MP4;
    }
    if (QStringLiteral("flac") == fileSuffix) {
        return FileType::FLAC;
    }
    if (QStringLiteral("ogg") == fileSuffix) {
        return FileType::OGG;
    }
    if (QStringLiteral("opus") == fileSuffix) {
        return FileType::OPUS;
    }
    if (QStringLiteral("wav") == fileSuffix) {
        return FileType::WAV;
    }
    if (QStringLiteral("wv") == fileSuffix) {
        return FileType::WV;
    }
    if (fileSuffix.startsWith(QStringLiteral("aif"))) {
        return FileType::AIFF;
    }
    return FileType::Unknown;
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
    const auto filename = file.name().wstr();
#else
    const auto filename = file.name();
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
