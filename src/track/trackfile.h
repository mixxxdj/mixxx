#pragma once

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QtDebug>

#include "util/fileinfo.h"

// Wrapper class for dealing with track files and their
// path/location, URL, and URI representations.
//
// All file-related track properties are snapshots from the provided
// QFileInfo. Obtaining them might involve accessing the file system
// and should be used consciously! The QFileInfo class does some
// caching behind the scenes.
//
// Please note that the canonical location of QFileInfo may change at
// any time, when the underlying file system structure is modified.
// It becomes empty if the file is deleted.
//
// Copying an instance of this class is thread-safe, because the
// underlying QFileInfo is implicitly shared.
//
// TODO: Replace with mixxx::FileInfo
class TrackFile {
  public:
    static TrackFile fromUrl(const QUrl& url) {
        return TrackFile(url.toLocalFile());
    }

    TrackFile() = default;
    // For backward-compatibility the QString single argument
    // constructor has not been declared as "explicit". It is
    // also not strictly necessary and might be removed at some
    // point to prevent unintended type conversions.
    /*non-explicit*/ TrackFile(
            const QString& filePath)
            : m_fileInfo(filePath) {
    }
    explicit TrackFile(
            QFileInfo fileInfo)
            : m_fileInfo(std::move(fileInfo)) {
    }
    explicit TrackFile(
            const QDir& dir,
            const QString& file = QString())
            : m_fileInfo(QFileInfo(dir, file)) {
    }
    explicit TrackFile(
            const mixxx::FileInfo& fileInfo)
            : m_fileInfo(fileInfo.asQFileInfo()) {
    }

    const QFileInfo& asFileInfo() const {
        return m_fileInfo;
    }

    // Local file representation
    QString location() const {
        return m_fileInfo.absoluteFilePath();
    }
    QString canonicalLocation() const {
        return m_fileInfo.canonicalFilePath();
    }
    QString directory() const {
        return m_fileInfo.absolutePath();
    }
    QString baseName() const {
        return m_fileInfo.baseName();
    }
    QString fileName() const {
        return m_fileInfo.fileName();
    }
    qint64 fileSize() const {
        return m_fileInfo.size();
    }

    QDateTime fileCreated() const {
        return m_fileInfo.birthTime();
    }
    QDateTime fileLastModified() const {
        return m_fileInfo.lastModified();
    }

    // Check if the file actually exists on the file system.
    bool checkFileExists() const {
        return QFile::exists(location());
    }

    void refresh() {
        m_fileInfo.refresh();
    }

    // Refresh the canonical location if it is still empty, i.e. if
    // the file may have re-appeared after mounting the corresponding
    // drive while Mixxx is already running.
    //
    // We ignore the case when the user changes a symbolic link to
    // point a file to an other location, since this is a user action.
    // We also don't care if a file disappears while Mixxx is running.
    // Opening a non-existent file is already handled and doesn't cause
    // any malfunction.
    QString freshCanonicalLocation();

    // Portable URL representation
    QUrl toUrl() const {
        return QUrl::fromLocalFile(location());
    }

    friend bool operator==(const TrackFile& lhs, const TrackFile& rhs) {
        return lhs.m_fileInfo == rhs.m_fileInfo;
    }

  private:
    QFileInfo m_fileInfo;
};

inline bool operator!=(const TrackFile& lhs, const TrackFile& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug debug, const TrackFile& trackFile) {
    return debug << trackFile.asFileInfo();
}

inline uint qHash(
        const TrackFile& key,
        uint seed = 0) {
    return qHash(key.location(), seed);
}
