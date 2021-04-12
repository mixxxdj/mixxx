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

/// A thin wrapper (shim) around QFileInfo with a limited API and
/// some additional methods.
///
/// Could be used as a drop-in replacement of QFileInfo with very
/// few exceptions where the name of member functions differ. Despite
/// the name it represents either a file, a directory, or a symbolic link.
///
/// This class adds support for the higher-level concept of a "location"
/// that is used to reference a permanent file path.
///
/// All single-argument are declared as explicit to prevent implicit conversions.
///
/// Implementation note: Inheriting from QFileInfo would violate the
/// Liskov Substition Principle. It is also invalid, because QFileInfo
/// has a non-virtual destructor and we cannot override non-virtual
/// member functions.
class FileInfo final {
  public:
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

    /// Directly access to the wrapped QFileInfo (immutable)
    const QFileInfo& asQFileInfo() const {
        return m_fileInfo;
    }

    /// Explicit conversion from QFile.
    static FileInfo fromQFile(const QFile& file) {
        return FileInfo(file);
    }

    /// Explicit conversion to QFile.
    QFile toQFile(QObject* parent = nullptr) const {
        return QFile(location(), parent);
    }

    /// Explicit conversion from QDir.
    static FileInfo fromQDir(const QDir& dir) {
        return FileInfo(dir);
    }

    /// Explicit conversion to QDir.
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

    /// Explicit conversion to a local file QUrl.
    QUrl toQUrl() const {
        return QUrl::fromLocalFile(location());
    }

    /// Check that the given QFileInfo is context-insensitive to avoid
    /// implicitly acccessing any transient working directory when
    /// resolving relative paths. We need to exclude these unintended
    /// side-effects!
    static bool hasLocation(const QFileInfo& fileInfo) {
        DEBUG_ASSERT(QFileInfo().isRelative()); // special case (should be excluded)
        return fileInfo.isAbsolute();
    }
    bool hasLocation() const {
        return hasLocation(m_fileInfo);
    }

    /// Returns the permanent location of a file.
    static QString location(const QFileInfo& fileInfo) {
        DEBUG_ASSERT(hasLocation(fileInfo));
        return fileInfo.absoluteFilePath();
    }
    QString location() const {
        return location(m_fileInfo);
    }

    /// The directory part of the location, i.e. excluding the file name.
    static QString locationPath(const QFileInfo& fileInfo) {
        DEBUG_ASSERT(hasLocation(fileInfo));
        return fileInfo.absolutePath();
    }
    QString locationPath() const {
        return locationPath(m_fileInfo);
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
    /// been deleted! A non-empty canonical location is immutable and
    /// does not disappear, other than by explicitly refresh()ing the
    /// file info manually. See also: FileInfoTest
    QString resolveCanonicalLocation();

    /// Returns the current location of a physical file, i.e.
    /// without aliasing by symbolic links and without any redundant
    /// relative paths.
    ///
    /// Does only access the file system if file metadata is not
    /// already cached, depending on the caching mode of QFileInfo.
    static QString canonicalLocation(const QFileInfo& fileInfo) {
        DEBUG_ASSERT(hasLocation(fileInfo));
        return fileInfo.canonicalFilePath();
    }
    QString canonicalLocation() const {
        return canonicalLocation(m_fileInfo);
    }

    /// The directory part of the canonical location, i.e. excluding
    /// the file name.
    ///
    /// Does only access the file system if file metadata is not
    /// already cached, depending on the caching mode of QFileInfo.
    static QString canonicalLocationPath(const QFileInfo& fileInfo) {
        DEBUG_ASSERT(hasLocation(fileInfo));
        return fileInfo.canonicalPath();
    }
    QString canonicalLocationPath() const {
        return canonicalLocationPath(m_fileInfo);
    }

    /// Decide if two canonical locations have a parent/child
    /// relationship, i.e. if the sub location is contained
    /// within the tree originating from the root location.
    ///
    /// Both canonical locations must not be empty, otherwise
    /// false is returned.
    static bool isRootSubCanonicalLocation(
            const QString& rootCanonicalLocation,
            const QString& subCanonicalLocation);

    /// Check if the file actually exists on the file system,
    /// bypassing any internal caching.
    bool checkFileExists() const {
        // Using filePath() is faster than location()
        return QFileInfo::exists(filePath());
    }

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

} // namespace mixxx

Q_DECLARE_TYPEINFO(mixxx::FileInfo, Q_MOVABLE_TYPE); // QFileInfo is movable
Q_DECLARE_METATYPE(mixxx::FileInfo)
