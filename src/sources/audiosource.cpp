#include "sources/audiosource.h"

#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("AudioSource");

} // anonymous namespace

AudioSource::AudioSource(const QUrl& url)
        : UrlResource(url) {
}

bool AudioSource::initBitrateOnce(Bitrate bitrate) {
    if (bitrate < Bitrate()) {
        kLogger.warning()
                << "Invalid bitrate"
                << bitrate;
        return false; // abort
    }
    VERIFY_OR_DEBUG_ASSERT(!m_bitrate.valid() || (m_bitrate == bitrate)) {
        kLogger.warning()
                << "Bitrate has already been initialized to"
                << m_bitrate
                << "which differs from"
                << bitrate;
        return false; // abort
    }
    m_bitrate = bitrate;
    return true;
}

bool AudioSource::verifyReadable() const {
    bool result = SampleFrameSource::verifyReadable();
    if (m_bitrate != Bitrate()) {
        VERIFY_OR_DEBUG_ASSERT(m_bitrate.valid()) {
            kLogger.warning()
                    << "Invalid bitrate [kbps]:"
                    << m_bitrate;
            // Don't set the result to false, because bitrate is only
            // an  informational property that does not effect the ability
            // to decode audio data!
        }
    }
    return result;
}

} // namespace mixxx
