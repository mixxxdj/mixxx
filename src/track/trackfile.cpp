#include "track/trackfile.h"


namespace {

inline
QUrl urlFromLocalFilePath(const QString& localFilePath) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    return QUrl::fromLocalFile(localFilePath);
#else
    if (localFilePath.isEmpty()) {
        return QUrl();
    } else {
        return QUrl::fromLocalFile(localFilePath);
    }
#endif
}

inline
QString urlToString(const QUrl& url) {
    return url.toEncoded(
            QUrl::StripTrailingSlash |
            QUrl::NormalizePathSegments);
}

} // anonymous namespace

QString TrackFile::freshCanonicalLocation() {
    // Note: We return here the cached value, that was calculated just after
    // init this TrackFile object. This will avoid repeated use of the time
    // consuming file IO.
    QString loc = canonicalLocation();
    if (loc.isEmpty()) {
        // Refresh the cached file info and try again
        m_fileInfo.refresh();
        loc = canonicalLocation();
    }
    return loc;
}

QUrl TrackFile::locationUrl() const {
    return urlFromLocalFilePath(location());
}

QUrl TrackFile::canonicalLocationUrl() const {
    return urlFromLocalFilePath(canonicalLocation());
}

QString TrackFile::locationUri() const {
    return urlToString(locationUrl());
}

QString TrackFile::canonicalLocationUri() const {
    return urlToString(canonicalLocationUrl());
}

QDebug operator<<(QDebug debug, const TrackFile& trackFile) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    return debug << static_cast<const QFileInfo&>(trackFile);
#else
    return debug << trackFile.location();
#endif
}
