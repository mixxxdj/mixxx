#include "sources/soundsource.h"

#include "track/trackmetadatataglib.h"

namespace mixxx {

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

SoundSource::OpenResult SoundSource::open(const AudioSourceConfig& audioSrcCfg) {
    close(); // reopening is not supported

    OpenResult result;
    try {
        result = tryOpen(audioSrcCfg);
    } catch (const std::exception& e) {
        qWarning() << "Caught unexpected exception from SoundSource::tryOpen():" << e.what();
        result = OpenResult::FAILED;
    } catch (...) {
        qWarning() << "Caught unknown exception from SoundSource::tryOpen()";
        result = OpenResult::FAILED;
    }
    if (OpenResult::SUCCEEDED != result) {
        close(); // rollback
    }
    return result;
}

Result SoundSource::parseTrackMetadataAndCoverArt(
        TrackMetadata* pTrackMetadata,
        QImage* pCoverArt) const {
    return taglib::readTrackMetadataAndCoverArtFromFile(pTrackMetadata, pCoverArt, getLocalFileName());
}

Result SoundSource::writeTrackMetadata(
        const TrackMetadata& trackMetadata) const {
    return taglib::writeTrackMetadataIntoFile(trackMetadata, getLocalFileName());
}

} //namespace mixxx
