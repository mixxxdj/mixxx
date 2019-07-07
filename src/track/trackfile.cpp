#include "track/trackfile.h"

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

QUrl TrackFile::toUrl() const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    return QUrl::fromLocalFile(location());
#else
    if (location().isEmpty()) {
        return QUrl();
    } else {
        return QUrl::fromLocalFile(location());
    }
#endif
}

QDebug operator<<(QDebug debug, const TrackFile& trackFile) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    return debug << trackFile.asFileInfo();
#else
    return debug << trackFile.location();
#endif
}
