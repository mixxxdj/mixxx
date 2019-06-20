#pragma once

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QtDebug>

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
class TrackFile {
  public:
    // For backward-compatibility the QString single argument
    // constructor has not been declared as "explicit". It is
    // also not strictly necessary and might be removed at some
    // point to prevent unintended type conversions.
    /*non-explicit*/ TrackFile(
            const QString& filePath)
            : m_fileInfo(filePath) {
    }
    explicit TrackFile(
            QFileInfo fileInfo = QFileInfo())
            : m_fileInfo(std::move(fileInfo)) {
    }
    explicit TrackFile(
            const QDir& dir,
            const QString& file = QString())
            : m_fileInfo(QFileInfo(dir, file)) {
    }

    operator QFileInfo&() {
        return m_fileInfo;
    }
    operator const QFileInfo&() const {
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
        return m_fileInfo.created();
    }
    QDateTime fileLastModified() const {
        return m_fileInfo.lastModified();
    }

    // Check if the file actually exists on the file system.
    bool checkFileExists() const {
        return QFile::exists(location());
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
    QUrl locationUrl() const;
    QUrl canonicalLocationUrl() const;

  private:
    QFileInfo m_fileInfo;
};

QDebug
operator<<(QDebug debug, const TrackFile& trackFile);
