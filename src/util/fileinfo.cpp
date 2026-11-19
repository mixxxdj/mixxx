#include "util/fileinfo.h"

#include <QHash>
#include <QMutex>
#include <QMutexLocker>

namespace {

// Memoized canonical locations of directories, keyed by their
// unresolved path. Resolving the canonical location of a file
// requires a component-wise file system lookup of every element
// in the path. On slow storage this repeated work dominates
// every other file access. Caching the result per directory
// limits the resolution to at most once per directory instead
// of once per file. Entries are only inserted after the
// directory has been resolved successfully.
QMutex s_canonicalDirLocationMutex;
QHash<QString, QString> s_canonicalDirLocations;

} // namespace

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
    // Resolving the canonical location of a file requires a
    // component-wise file system lookup of every element in the path.
    // On slow storage like network file systems this repeated work
    // dominates every other file access. If the canonical location of
    // the containing directory has already been resolved once, the
    // canonical location of the file itself can be composed from it.
    // A leaf component that either does not exist or is a symbolic link
    // still needs the full resolution below.
    const QString dirPath = m_fileInfo.path();
    const QString fileName = m_fileInfo.fileName();
    if (!fileName.isEmpty() && QDir::isAbsolutePath(dirPath)) {
        QString canonicalDirLocation;
        {
            const QMutexLocker locker(&s_canonicalDirLocationMutex);
            canonicalDirLocation = s_canonicalDirLocations.value(dirPath);
        }
        if (canonicalDirLocation.isEmpty()) {
            canonicalDirLocation = QFileInfo(dirPath).canonicalFilePath();
            if (!canonicalDirLocation.isEmpty()) {
                const QMutexLocker locker(&s_canonicalDirLocationMutex);
                s_canonicalDirLocations.insert(dirPath, canonicalDirLocation);
            }
        }
        if (!canonicalDirLocation.isEmpty()) {
            QString location = canonicalDirLocation;
            if (!location.endsWith('/')) {
                location += '/';
            }
            location += fileName;
            const QFileInfo candidateInfo(location);
            if (candidateInfo.exists() && !candidateInfo.isSymLink()) {
                // The file info needs to be refreshed before returning
                // like on the slow path below to ensure that all other
                // file properties are considered fresh afterwards.
                m_fileInfo.refresh();
                return location;
            }
        }
    }
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
