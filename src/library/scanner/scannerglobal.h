#ifndef SCANNERGLOBAL_H
#define SCANNERGLOBAL_H

#include <QSet>
#include <QHash>
#include <QRegExp>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>

#include "util/task.h"
#include "util/performancetimer.h"

class DirInfo {
  public:
    DirInfo(const QDir& dir,
            const SecurityTokenPointer& token)
          : m_dir(dir),
            m_token(token) {
    }

    const QDir& dir() const {
        return m_dir;
    }

    const SecurityTokenPointer& token() const {
        return m_token;
    }

  private:
    QDir m_dir;
    SecurityTokenPointer m_token;
};


class ScannerGlobal {
  public:
    ScannerGlobal(const QSet<QString>& trackLocations,
                  const QHash<QString, int>& directoryHashes,
                  const QRegExp& supportedExtensionsMatcher,
                  const QRegExp& supportedCoverExtensionsMatcher,
                  const QStringList& directoriesBlacklist)
            : m_trackLocations(trackLocations),
              m_directoryHashes(directoryHashes),
              m_supportedExtensionsMatcher(supportedExtensionsMatcher),
              m_supportedCoverExtensionsMatcher(supportedCoverExtensionsMatcher),
              m_directoriesBlacklist(directoriesBlacklist),
              // Unless marked un-clean, we assume it will finish cleanly.
              m_scanFinishedCleanly(true),
              m_shouldCancel(false),
              m_numScannedDirectories(0) {
    }

    TaskWatcher& getTaskWatcher() {
        return m_watcher;
    }

    // Returns whether the track already exists in the database.
    inline bool trackExistsInDatabase(const QString& trackLocation) const {
        return m_trackLocations.contains(trackLocation);
    }

    // Returns the directory hash if it exists or -1 if it doesn't.
    inline int directoryHashInDatabase(const QString& directoryPath) const {
        return m_directoryHashes.value(directoryPath, -1);
    }

    inline bool directoryBlacklisted(const QString& directoryPath) const {
        return m_directoriesBlacklist.contains(directoryPath);
    }

    const QRegExp& supportedExtensionsRegex() const {
        return m_supportedExtensionsMatcher;
    }

    inline bool testAndMarkDirectoryScanned(const QDir& dir) {
        const QString canonicalPath(dir.canonicalPath());
        QMutexLocker locker(&m_directoriesScannedMutex);
        if (m_directoriesScanned.contains(canonicalPath)) {
            return true;
        } else {
            m_directoriesScanned.insert(canonicalPath);
            return false;
        }
    }

    inline void addUnhashedDir(const QDir& dir,
                               const SecurityTokenPointer& token) {
        QMutexLocker locker(&m_directoriesUnhashedMutex);
        m_directoriesUnhashed.append(DirInfo(dir, token));
    }

    inline QList<DirInfo>& unhashedDirs() {
        // no need for locking here, because it is only used
        // when only one using thread is around.
        return m_directoriesUnhashed;
    }

    // TODO(rryan) test whether tasks should create their own QRegExp.
    inline bool isAudioFileSupported(const QString& fileName) const {
        QMutexLocker locker(&m_supportedExtensionsMatcherMutex);
        return m_supportedExtensionsMatcher.indexIn(fileName) != -1;
    }

    const QRegExp& supportedCoverExtensionsRegex() const {
        return m_supportedCoverExtensionsMatcher;
    }

    // TODO(rryan) test whether tasks should create their own QRegExp.
    inline bool isCoverFileSupported(const QString& fileName) const {
        QMutexLocker locker(&m_supportedCoverExtensionsMatcherMutex);
        return m_supportedCoverExtensionsMatcher.indexIn(fileName) != -1;
    }

    inline bool shouldCancel() const {
        return m_shouldCancel;
    }

    inline volatile const bool* shouldCancelPointer() const {
        return &m_shouldCancel;
    }

    void cancel() {
        m_shouldCancel = true;
    }

    inline bool scanFinishedCleanly() const {
        return m_scanFinishedCleanly;
    }

    void clearScanFinishedCleanly() {
        m_scanFinishedCleanly = false;
    }

    void addVerifiedDirectory(const QString& directory) {
        m_verifiedDirectories << directory;
    }

    const QStringList& verifiedDirectories() const {
        return m_verifiedDirectories;
    }

    void addVerifiedTrack(const QString& trackLocation) {
        m_verifiedTracks << trackLocation;
    }

    const QStringList& verifiedTracks() const {
        return m_verifiedTracks;
    }

    void startTimer() {
        m_timer.start();
    }

    // Elapsed time in nanoseconds since startTimer was called.
    qint64 timerElapsed() {
        return m_timer.elapsed();
    }

    const QStringList& addedTracks() const {
        return m_addedTracks;
    }
    void trackAdded(const QString& trackLocation) {
        m_addedTracks << trackLocation;
    }

    int numScannedDirectories() const {
        return m_numScannedDirectories;
    }
    void directoryScanned() {
        m_numScannedDirectories++;
    }


  private:
    TaskWatcher m_watcher;

    QSet<QString> m_trackLocations;
    QHash<QString, int> m_directoryHashes;

    mutable QMutex m_supportedExtensionsMatcherMutex;
    QRegExp m_supportedExtensionsMatcher;

    mutable QMutex m_supportedCoverExtensionsMatcherMutex;
    QRegExp m_supportedCoverExtensionsMatcher;

    // This set will grow during a scan by successively
    // inserting the canonical paths of directories that
    // are about to be scanned.
    mutable QMutex m_directoriesScannedMutex;
    QSet<QString> m_directoriesScanned;

    // This set will collect all locations of new
    // discovered directories, they are scanned in a
    // second run to avoid swapping between duplicated tracks
    mutable QMutex m_directoriesUnhashedMutex;
    QList<DirInfo> m_directoriesUnhashed;

    // Typically there are 1 to 2 entries in the blacklist so a O(n) search in a
    // QList may have better constant factors than a O(1) QSet check. However,
    // this has never been investigated.
    QStringList m_directoriesBlacklist;

    // The list of directories verified by the scan.
    QStringList m_verifiedDirectories;

    // The list of tracks verified by the scan.
    QStringList m_verifiedTracks;

    // The list of tracks added by the scan.
    QStringList m_addedTracks;

    volatile bool m_scanFinishedCleanly;
    volatile bool m_shouldCancel;

    // Stats tracking.
    PerformanceTimer m_timer;
    int m_numScannedDirectories;
};

typedef QSharedPointer<ScannerGlobal> ScannerGlobalPointer;

#endif /* SCANNERGLOBAL_H */
