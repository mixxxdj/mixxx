#include "sources/soundsource.h"

#include <QMimeDatabase>
#include <QMimeType>

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSource");

inline QUrl validateLocalFileUrl(QUrl url) {
    DEBUG_ASSERT(url.isValid());
    VERIFY_OR_DEBUG_ASSERT(url.isLocalFile()) {
        kLogger.warning()
                << "Unsupported URL:"
                << url.toString();
    }
    return url;
}

} // anonymous namespace

//static
QString SoundSource::getTypeFromUrl(const QUrl& url) {
    const QString filePath = validateLocalFileUrl(url).toLocalFile();
    return getTypeFromFile(filePath);
}

//static
QString SoundSource::getTypeFromFile(const QFileInfo& fileInfo) {
    const QString fileSuffix = fileInfo.suffix();
    DEBUG_ASSERT(!fileSuffix.isEmpty() || fileSuffix == QString{});
    const QMimeType mimeType = QMimeDatabase().mimeTypeForFile(fileInfo);
    if (!mimeType.isValid()) {
        qWarning()
                << "Unknown MIME type for file" << fileInfo.filePath();
        return fileSuffix;
    }
    const QString preferredSuffix = mimeType.preferredSuffix();
    if (preferredSuffix.isEmpty()) {
        DEBUG_ASSERT(mimeType.suffixes().isEmpty());
        qInfo()
                << "MIME type" << mimeType
                << "has no preferred suffix";
        return fileSuffix;
    }
    if (fileSuffix == preferredSuffix || mimeType.suffixes().contains(fileSuffix)) {
        return fileSuffix;
    }
    if (fileSuffix.isEmpty()) {
        qWarning()
                << "Using type" << preferredSuffix
                << "according to the detected MIME type" << mimeType
                << "of file" << fileInfo.filePath();
    } else {
        qWarning()
                << "Using type" << preferredSuffix
                << "instead of" << fileSuffix
                << "according to the detected MIME type" << mimeType
                << "of file" << fileInfo.filePath();
    }
    return preferredSuffix;
}

SoundSource::SoundSource(const QUrl& url, const QString& type)
        : AudioSource(validateLocalFileUrl(url)),
          MetadataSourceTagLib(getLocalFileName()),
          m_type(type) {
}

} // namespace mixxx
