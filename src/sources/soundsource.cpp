#include "sources/soundsource.h"

#include <QMimeDatabase>
#include <QMimeType>

#include "sources/soundsourceproxy.h"
#include "track/steminfoimporter.h"
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
    return getTypeFromFile(QFileInfo(filePath));
}

//static
QString SoundSource::getTypeFromFile(const QFileInfo& fileInfo) {
    const QString fileSuffix = fileInfo.suffix().toLower().trimmed();

    if (fileSuffix == QLatin1String("opus")) {
        // Bypass the insufficient mime type lookup from content for opus files
        // Files with "opus" suffix are of mime type "audio/x-opus+ogg" or "audio/opus".
        // In case of "audio/x-opus+ogg" only the container format "audio/ogg"
        // is detected, which will be decoded with the SoundSourceOggVobis()
        // but we want SoundSourceOpus(). "audio/opus" files without ogg
        // container are detected as "text/plain". They are not yet supported by Mixxx.
        return fileSuffix;
    }
    if (fileSuffix == QLatin1String("flac")) {
        // Bypass the insufficient mime type lookup from content for FLAC files.
        // Legacy FLAC files may contain an ID3 tag (written by ExactAudioCopy and
        // others) that causes these files to be identified as "audio/mpeg" instead
        // of "audio/flac". Most decoders and TagLib are able to ignore and skip
        // the non-standard ID3 data.
        // https://mixxx.zulipchat.com/#narrow/stream/109171-development/topic/mimetype.20sometimes.20wrong
        return fileSuffix;
    }
    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(
            fileInfo, QMimeDatabase::MatchContent);
#ifdef __STEM__
    if (
            // STEM files will be detected as normal MP4, so we check if the file
            // is looking like a MP4
            StemInfoImporter::maybeStemFile(fileInfo.filePath(), mimeType) &&
            // If yes, we search a STEM atom and assume they are valid STEM file
            // if they do contain one
            StemInfoImporter::hasStemAtom(fileInfo.filePath())) {
        return QLatin1String("stem.mp4");
    }
#endif

    // According to the documentation mimeTypeForFile always returns a valid
    // type, using the generic type application/octet-stream as a fallback.
    // This might also occur for missing files as seen on Qt 5.12.
    if (!mimeType.isValid() || mimeType.isDefault()) {
        if (fileInfo.exists()) {
            qInfo() << "Unable to detect MIME type from file" << fileInfo.filePath();
        } else {
            qInfo() << "Unable to detect MIME type from not existing file" << fileInfo.filePath();
        }
        mimeType = QMimeDatabase().mimeTypeForFile(
                fileInfo, QMimeDatabase::MatchExtension);
        if (!mimeType.isValid() || mimeType.isDefault()) {
            return fileSuffix;
        }
    }
    const QString fileType = SoundSourceProxy::getFileTypeByMimeType(mimeType);
    if (fileType.isEmpty()) {
        qWarning() << "No file type registered for MIME type" << mimeType;
        return fileSuffix;
    }
    if (fileType != fileSuffix && !mimeType.suffixes().contains(fileSuffix)) {
        qWarning()
                << "Using type" << fileType
                << "instead of" << fileSuffix
                << "according to the detected MIME type" << mimeType
                << "of file" << fileInfo.filePath();
    }
    return fileType;
}

SoundSource::SoundSource(const QUrl& url, const QString& type)
        : AudioSource(validateLocalFileUrl(url)),
          MetadataSourceTagLib(getLocalFileName(), type),
          m_type(type) {
}

} // namespace mixxx
