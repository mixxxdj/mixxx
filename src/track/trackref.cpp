#include "track/trackref.h"


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

//static
QUrl TrackRef::locationUrl(const QFileInfo& fileInfo) {
    return urlFromLocalFilePath(location(fileInfo));
}

//static
QUrl TrackRef::canonicalLocationUrl(const QFileInfo& fileInfo) {
    return urlFromLocalFilePath(canonicalLocation(fileInfo));
}

//static
QString TrackRef::locationUri(const QFileInfo& fileInfo) {
    return urlToString(locationUrl(fileInfo));
}

//static
QString TrackRef::canonicalLocationUri(const QFileInfo& fileInfo) {
    return urlToString(canonicalLocationUrl(fileInfo));
}

bool TrackRef::verifyConsistency() const {
    // Class invariant: The location can only be set together with
    // at least one of the other members!
    VERIFY_OR_DEBUG_ASSERT(!hasCanonicalLocation() || hasLocation()) {
        // Condition violated: hasCanonicalLocation() => hasLocation()
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(!hasId() || hasLocation()) {
        // Condition violated: hasId() => hasLocation()
        return false;
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const TrackRef& trackRef) {
    return os << '[' << trackRef.getLocation().toStdString()
            << " | " << trackRef.getCanonicalLocation().toStdString()
            << " | " << trackRef.getId()
            << ']';

}

QDebug operator<<(QDebug debug, const TrackRef& trackRef) {
    debug.nospace() << '[' << trackRef.getLocation()
                    << " | " << trackRef.getCanonicalLocation()
                    << " | " << trackRef.getId()
                    << ']';
    return debug.space();
}
