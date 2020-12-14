#pragma once

#include <gtest/gtest_prod.h>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QtDebug>

#include "util/assert.h"

namespace mixxx {

/// Helper functions and extensions for QFileInfo using Mixxx terminology.
namespace fileinfo {

/// Only intended for internal usage.
inline const QFileInfo& assertNotRelative(const QFileInfo& fileInfo) {
    // Detect potential, unintended side-effects by the outer context,
    // i.e. when accessing the transient working directory to resolve
    // of relative paths. The path should either be empty or absolute,
    // i.e. not relative.
    DEBUG_ASSERT(!fileInfo.isRelative());
    return fileInfo;
}

/// Returns the permanent location of a file.
inline QString location(const QFileInfo& fileInfo) {
    return assertNotRelative(fileInfo).absoluteFilePath();
}

/// The directory part of the location, i.e. excluding the file name.
inline QString locationPath(const QFileInfo& fileInfo) {
    return assertNotRelative(fileInfo).absolutePath();
}

/// Returns the current location of a physical file, i.e.
/// without aliasing by symbolic links and without any redundant
/// relative paths.
inline QString canonicalLocation(const QFileInfo& fileInfo) {
    return assertNotRelative(fileInfo).canonicalFilePath();
}

/// The directory part of the canonical location, i.e. excluding the file name.
inline QString canonicalLocationPath(const QFileInfo& fileInfo) {
    return assertNotRelative(fileInfo).canonicalPath();
}

/// Decide if two canonical locations have a parent/child
/// relationship, i.e. if the sub location is contained
/// within the tree originating from the root location.
///
/// Both canonical locations must not be empty, otherwise
/// false is returned.
bool isRootSubCanonicalLocation(
        const QString& rootCanonicalLocation,
        const QString& subCanonicalLocation);

} // namespace fileinfo

/// A thin wrapper (shim) around QFileInfo with additional methods.
///
/// Could be used as a drop-in replacement of QFileInfo with very
/// few exceptions where the name of member functions differ. Despite
/// the name it represents either a file, a directory, or a symbolic link.
///
/// It adds support for the concept of a "location" that is used
/// to persistently reference a permanent file path. The location
/// is used for calculating a hash key with qHash().
class FileInfo final {
  public:
    /////////////////////////////////////////////////////////////////////////
    /// Construction & Assignment
    ///
    /// Constructor arguments are passed to QFileInfo. All single-argument
    /// are declared as explicit to prevent implicit conversions.
    /////////////////////////////////////////////////////////////////////////
    explicit FileInfo(
            QFileInfo&& fileInfo)
            : m_fileInfo(std::move(fileInfo)) {
    }
    explicit FileInfo(
            const QFileInfo& fileInfo)
            : m_fileInfo(fileInfo) {
    }
    explicit FileInfo(
            const QFile& file)
            : m_fileInfo(file) {
    }
    explicit FileInfo(
            const QString& file)
            : m_fileInfo(file) {
    }
    explicit FileInfo(
            const QDir& dir,
            const QString& file = QString())
            : m_fileInfo(dir, file) {
    }
    FileInfo() = default;
    FileInfo(FileInfo&&) = default;
    FileInfo(const FileInfo&) = default;
    FileInfo& operator=(FileInfo&&) = default;
    FileInfo& operator=(const FileInfo&) = default;

    /////////////////////////////////////////////////////////////////////////
    /// Direct access to the wrapped QFileInfo (immutable)
    /////////////////////////////////////////////////////////////////////////
    const QFileInfo& asQFileInfo() const {
        return m_fileInfo;
    }

    /////////////////////////////////////////////////////////////////////////
    /// Explicit conversion from/to QFile/QDir/QUrl
    /////////////////////////////////////////////////////////////////////////

    /// Explicit conversion from QFile.
    static FileInfo fromQFile(const QFile& file) {
        return FileInfo(file);
    }

    QFile toQFile(QObject* parent = nullptr) const {
        return QFile(location(), parent);
    }

    /// Explicit conversion from QDir.
    static FileInfo fromQDir(const QDir& dir) {
        return FileInfo(dir);
    }

    QDir toQDir() const {
        // Due to false negatives we must assert for !isFile() instead
        // of isDir()!
        DEBUG_ASSERT(!isFile());
        // We cannot use QFileInfo::dir() which instead returns the
        // parent directory.
        return QDir(location());
    }

    /// Explicit conversion from a local file QUrl.
    static FileInfo fromQUrl(const QUrl& url) {
        return FileInfo(url.toLocalFile());
    }

    QUrl toQUrl() const {
        return QUrl::fromLocalFile(location());
    }

    /////////////////////////////////////////////////////////////////////////
    /// Location
    /////////////////////////////////////////////////////////////////////////

    QString location() const {
        return fileinfo::location(m_fileInfo);
    }

    QString locationPath() const {
        return fileinfo::locationPath(m_fileInfo);
    }

    QString canonicalLocation() const {
        return fileinfo::canonicalLocation(m_fileInfo);
    }

    QString canonicalLocationPath() const {
        return fileinfo::canonicalLocationPath(m_fileInfo);
    }

    /// Refresh the canonical location if it is still empty, i.e. if
    /// the file may have re-appeared after mounting the corresponding
    /// drive while Mixxx is already running.
    ///
    /// We ignore the case when the user changes a symbolic link to
    /// point a file to an other location, since this is a user action.
    /// We also don't care if a file disappears while Mixxx is running.
    /// Opening a non-existent file is already handled and doesn't cause
    /// any malfunction.
    ///
    /// Note: Refreshing will never invalidate the canonical location
    /// once it has been set, even after the corresponding file has
    /// been deleted! A non-empty canonical location is immutable.
    /// See also: FileInfoTest
    QString freshCanonicalLocation();

    /////////////////////////////////////////////////////////////////////////
    /// File system
    /////////////////////////////////////////////////////////////////////////

    /// Check if the file actually exists on the file system,
    /// bypassing any internal caching.
    bool checkFileExists() const {
        // Using filePath() is faster than location()
        return QFileInfo::exists(filePath());
    }

    /////////////////////////////////////////////////////////////////////////
    /// QFileInfo pass-through
    ///
    /// Selected member function from QFileInfo that are exposed by the
    /// public API with deliberate exceptions.
    ///
    /// All functions that refer to the path are hidden! Instead the
    /// location should be used to avoid inconsistencies.
    /////////////////////////////////////////////////////////////////////////

    void refresh() {
        m_fileInfo.refresh();
    }

    bool exists() const {
        return m_fileInfo.exists();
    }

    QString fileName() const {
        return m_fileInfo.fileName();
    }
    QString baseName() const {
        return m_fileInfo.baseName();
    }

    QDateTime birthTime() const {
        return m_fileInfo.birthTime();
    }

    QDateTime lastModified() const {
        return m_fileInfo.lastModified();
    }

    // Both isFile() and isDir() might return false, but they
    // will never return true at the same time, i.e. consider
    // false negatives when using these functions.
    bool isFile() const {
        return m_fileInfo.isFile();
    }
    bool isDir() const {
        return m_fileInfo.isDir();
    }

    bool isReadable() const {
        return m_fileInfo.isReadable();
    }
    bool isWritable() const {
        return m_fileInfo.isWritable();
    }

    bool isAbsolute() const {
        return m_fileInfo.isAbsolute();
    }
    /// Note: An empty file path is relative!
    bool isRelative() const {
        return m_fileInfo.isRelative();
    }

    QString suffix() const {
        return m_fileInfo.suffix();
    }
    QString completeSuffix() const {
        return m_fileInfo.completeSuffix();
    }

    /// Query the file size in bytes.
    ///
    /// Note: The longer name of this method compared to QFileInfo::size()
    /// has been chosen deliberately.
    qint64 sizeInBytes() const {
        return m_fileInfo.size();
    }

    friend bool operator==(const FileInfo& lhs, const FileInfo& rhs) {
        return lhs.m_fileInfo == rhs.m_fileInfo;
    }

    friend QDebug operator<<(QDebug dbg, const mixxx::FileInfo& arg) {
        return dbg << arg.m_fileInfo;
    }

  private:
    FRIEND_TEST(FileInfoTest, freshCanonicalFileInfo);

    // The internal file path should only be used for implementation
    // purpose. Using location() instead of filePath() is recommended
    // for all public use cases.
    QString filePath() const {
        return m_fileInfo.filePath();
    }

    QFileInfo m_fileInfo;
};

inline bool operator!=(const FileInfo& lhs, const FileInfo& rhs) {
    return !(lhs == rhs);
}

inline uint qHash(
        const FileInfo& key,
        uint seed = 0) {
    return qHash(key.location(), seed);
}

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::FileInfo, Q_MOVABLE_TYPE); // QFileInfo is movable
Q_DECLARE_METATYPE(mixxx::FileInfo)
