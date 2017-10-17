#include "sources/soundsource.h"

#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("AudioSource");

inline
QUrl validateUrl(QUrl url) {
    DEBUG_ASSERT(url.isValid());
    VERIFY_OR_DEBUG_ASSERT(url.isLocalFile()) {
        kLogger.warning()
                << "Unsupported URL:"
                << url.toString();
    }
    return url;
}

} // anonymous namespace

/*static*/ QString SoundSource::getFileExtensionFromUrl(QUrl url) {
    return validateUrl(url).toString().section(".", -1).toLower().trimmed();
}

SoundSource::SoundSource(QUrl url, QString type)
        : AudioSource(validateUrl(url)),
          TracklibMetadataSource(getLocalFileName()),
          m_type(type) {
}

SoundSource::OpenResult SoundSource::open(
        OpenMode mode,
        const AudioSourceConfig& audioSrcCfg) {
    close(); // reopening is not supported

    OpenResult result;
    try {
        result = tryOpen(mode, audioSrcCfg);
    } catch (const std::exception& e) {
        qWarning() << "Caught unexpected exception from SoundSource::tryOpen():" << e.what();
        result = OpenResult::Failed;
    } catch (...) {
        qWarning() << "Caught unknown exception from SoundSource::tryOpen()";
        result = OpenResult::Failed;
    }
    if (OpenResult::Succeeded != result) {
        close(); // rollback
    }
    return result;
}

} //namespace mixxx
