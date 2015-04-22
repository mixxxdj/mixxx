#include "sources/soundsource.h"

#include "metadata/trackmetadatataglib.h"

namespace Mixxx {

/*static*/ QString SoundSource::getTypeFromUrl(QUrl url) {
    return url.toString().section(".", -1).toLower().trimmed();
}

SoundSource::SoundSource(QUrl url)
        : AudioSource(url),
          m_type(getTypeFromUrl(url)) {
    DEBUG_ASSERT(getUrl().isValid());
}

SoundSource::SoundSource(QUrl url, QString type)
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

Result SoundSource::parseTrackMetadata(Mixxx::TrackMetadata* pMetadata) const {
    return readTrackMetadataFromFile(pMetadata, getLocalFileName());
}

QImage SoundSource::parseCoverArt() const {
    QImage coverArt;
    readCoverArtFromFile(&coverArt, getLocalFileName());
    return coverArt;
}

} //namespace Mixxx
