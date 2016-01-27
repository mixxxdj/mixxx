#include "sources/soundsource.h"

#include "track/trackmetadatataglib.h"

namespace Mixxx {

/*static*/ QString SoundSource::getFileExtensionFromUrl(const QUrl& url) {
    return url.toString().section(".", -1).toLower().trimmed();
}

SoundSource::SoundSource(const QUrl& url)
        : AudioSource(url),
          // simply use the file extension as the type
          m_type(getFileExtensionFromUrl(url)) {
    DEBUG_ASSERT(getUrl().isValid());
}

SoundSource::SoundSource(const QUrl& url, const QString& type)
        : AudioSource(url),
          m_type(type) {
    DEBUG_ASSERT(getUrl().isValid());
}

Result SoundSource::open(const AudioSourceConfig& audioSrcCfg) {
    close(); // reopening is not supported
    Result result;
    try {
        result = tryOpen(audioSrcCfg);
    } catch (...) {
        close();
        throw;
    }
    if (OK != result) {
        close();
    }
    return result;
}

Result SoundSource::parseTrackMetadataAndCoverArt(
        TrackMetadata* pTrackMetadata,
        QImage* pCoverArt) const {
    return readTrackMetadataAndCoverArtFromFile(pTrackMetadata, pCoverArt, getLocalFileName());
}

Result SoundSource::writeTrackMetadata(
        const TrackMetadata& trackMetadata) const {
    return writeTrackMetadataIntoFile(trackMetadata, getLocalFileName());
}

} //namespace Mixxx
