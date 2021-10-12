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

inline QString fileTypeFromSuffix(const QString& suffix) {
    const QString fileType = suffix.toLower().trimmed();
    if (fileType.isEmpty()) {
        // Always return a default-constructed, null string instead
        // of an empty string which might either be null or "".
        return QString{};
    }
    // Map shortened suffix "aif" to "aiff" for disambiguation
    if (fileType == QStringLiteral("aif")) {
        return QStringLiteral("aiff");
    }
    return fileType;
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
    const QString fileType = fileTypeFromSuffix(fileSuffix);
    DEBUG_ASSERT(!fileType.isEmpty() || fileType == QString{});
    const QMimeType mimeType = QMimeDatabase().mimeTypeForFile(fileInfo);
    // According to the documentation mimeTypeForFile always returns a valid
    // type, using the generic type application/octet-stream as a fallback.
    // This might also occur for missing files as seen on Qt 5.12.
    if (!mimeType.isValid() ||
            mimeType.name() == QStringLiteral("application/octet-stream")) {
        qWarning()
                << "Unknown MIME type for file" << fileInfo.filePath();
        return fileType;
    }
    const QString preferredSuffix = mimeType.preferredSuffix();
    if (preferredSuffix.isEmpty()) {
        DEBUG_ASSERT(mimeType.suffixes().isEmpty());
        qInfo()
                << "MIME type" << mimeType
                << "has no preferred suffix";
        return fileType;
    }
    const QString preferredFileType = fileTypeFromSuffix(preferredSuffix);
    if (fileType == preferredFileType || mimeType.suffixes().contains(fileSuffix)) {
        return fileType;
    }
    if (fileType.isEmpty()) {
        qWarning()
                << "Using type" << preferredFileType
                << "according to the detected MIME type" << mimeType
                << "of file" << fileInfo.filePath();
    } else {
        qWarning()
                << "Using type" << preferredFileType
                << "instead of" << fileType
                << "according to the detected MIME type" << mimeType
                << "of file" << fileInfo.filePath();
    }
    return preferredFileType;
}

SoundSource::SoundSource(const QUrl& url, const QString& type)
        : AudioSource(validateLocalFileUrl(url)),
          MetadataSourceTagLib(getLocalFileName()),
          m_type(type) {
}

} // namespace mixxx
