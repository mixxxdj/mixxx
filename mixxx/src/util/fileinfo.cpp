#include "util/fileinfo.h"

namespace mixxx {

// static
bool FileInfo::isRootSubCanonicalLocation(
        const QString& rootCanonicalLocation,
        const QString& subCanonicalLocation) {
    VERIFY_OR_DEBUG_ASSERT(!rootCanonicalLocation.isEmpty()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(!subCanonicalLocation.isEmpty()) {
        return false;
    }
    DEBUG_ASSERT(QDir::isAbsolutePath(rootCanonicalLocation));
    DEBUG_ASSERT(QDir::isAbsolutePath(subCanonicalLocation));
    if (subCanonicalLocation.size() < rootCanonicalLocation.size()) {
        return false;
    }
    if (subCanonicalLocation.size() > rootCanonicalLocation.size() &&
            subCanonicalLocation[rootCanonicalLocation.size()] != QChar('/')) {
        return false;
    }
    return subCanonicalLocation.startsWith(rootCanonicalLocation);
}

QString FileInfo::resolveCanonicalLocation() {
    // Note: We return here the cached value, that was calculated just after
    // init this FileInfo object. This will avoid repeated use of the time
    // consuming file I/O.
    QString currentCanonicalLocation = canonicalLocation();
    if (!currentCanonicalLocation.isEmpty()) {
        return currentCanonicalLocation;
    }
    m_fileInfo.refresh();
    // Return whatever is available after the refresh
    return canonicalLocation();
}

} // namespace mixxx
