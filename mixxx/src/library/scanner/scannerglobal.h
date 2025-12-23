#pragma once

#include <QDir>
#include <QHash>
#include <QMutex>
#include <QRegularExpression>
#include <QSet>
#include <QSharedPointer>
#include <QStringList>

#include "util/cache.h"
#include "util/compatibility/qmutex.h"
#include "util/fileaccess.h"
#include "util/performancetimer.h"
#include "util/task.h"

class ScannerGlobal {
  public:
    ScannerGlobal(const QSet<QString>& trackLocations,
            const QHash<QString, mixxx::cache_key_t>& directoryHashes,
            const QRegularExpression& supportedExtensionsMatcher,
            const QRegularExpression& supportedCoverExtensionsMatcher,
            const QStringList& directoriesBlacklist)
            : m_trackLocations(trackLocations),
              m_directoryHashes(directoryHashes),
              m_supportedExtensionsMatcher(supportedExtensionsMatcher),
              m_supportedCoverExtensionsMatcher(supportedCoverExtensionsMatcher),
              m_directoriesBlacklist(directoriesBlacklist),
              // Unless marked un-clean, we assume it will finish cleanly.
              m_scanFinishedCleanly(true),
              m_shouldCancel(false),
              m_numScannedDirectories(0),
              m_numRelocatedTracks(0) {
    }

    TaskWatcher& getTaskWatcher() {
        return m_watcher;
    }

    // Returns whether the track already exists in the database.
    bool trackExistsInDatabase(const QString& trackLocation) const {
        return m_trackLocations.contains(trackLocation);
    }

    // Returns the directory hash if it exists or mixxx::invalidCacheKey() if it doesn't.
    mixxx::cache_key_t directoryHashInDatabase(const QString& directoryPath) const {
        return m_directoryHashes.value(directoryPath, mixxx::invalidCacheKey());
    }

    bool directoryBlacklisted(const QString& directoryPath) const {
        return m_directoriesBlacklist.contains(directoryPath);
    }

    const QRegularExpression& supportedExtensionsRegex() const {
        return m_supportedExtensionsMatcher;
    }

    bool testAndMarkDirectoryScanned(const QDir& dir) {
        const QString canonicalPath(dir.canonicalPath());
        const auto locker = lockMutex(&m_directoriesScannedMutex);
        if (m_directoriesScanned.contains(canonicalPath)) {
            return true;
        } else {
            m_directoriesScanned.insert(canonicalPath);
            return false;
        }
    }

    void addUnhashedDir(const mixxx::FileAccess& dirAccess) {
        const auto locker = lockMutex(&m_directoriesUnhashedMutex);
        m_directoriesUnhashed.append(dirAccess);
    }

    const QList<mixxx::FileAccess>& unhashedDirs() const {
        // no need for locking here, because it is only used
        // when only one using thread is around.
        return m_directoriesUnhashed;
    }

    // TODO(rryan) test whether tasks should create their own QRegularExpression.
    bool isAudioFileSupported(const QString& fileName) const {
        const auto locker = lockMutex(&m_supportedExtensionsMatcherMutex);
        QRegularExpressionMatch match = m_supportedCoverExtensionsMatcher.match(fileName);
        return match.hasMatch();
    }

    const QRegularExpression& supportedCoverExtensionsRegex() const {
        return m_supportedCoverExtensionsMatcher;
    }

    // TODO(rryan) test whether tasks should create their own QRegularExpression.
    bool isCoverFileSupported(const QString& fileName) const {
        const auto locker = lockMutex(&m_supportedCoverExtensionsMatcherMutex);
        QRegularExpressionMatch match = m_supportedCoverExtensionsMatcher.match(fileName);
        return match.hasMatch();
    }

    bool shouldCancel() const {
        return m_shouldCancel;
    }

    volatile const bool* shouldCancelPointer() const {
        return &m_shouldCancel;
    }

    void cancel() {
        m_shouldCancel = true;
    }

    bool scanFinishedCleanly() const {
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

    // Elapsed time since startTimer was called.
    mixxx::Duration timerElapsed() {
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

    int numRelocatedTracks() const {
        return m_numRelocatedTracks;
    }
    void addRelocatedTracks(int numTracks) {
        m_numRelocatedTracks += numTracks;
    }

  private:
    TaskWatcher m_watcher;

    QSet<QString> m_trackLocations;
    QHash<QString, mixxx::cache_key_t> m_directoryHashes;

    mutable QMutex m_supportedExtensionsMatcherMutex;
    QRegularExpression m_supportedExtensionsMatcher;

    mutable QMutex m_supportedCoverExtensionsMatcherMutex;
    QRegularExpression m_supportedCoverExtensionsMatcher;

    // This set will grow during a scan by successively
    // inserting the canonical paths of directories that
    // are about to be scanned.
    mutable QMutex m_directoriesScannedMutex;
    QSet<QString> m_directoriesScanned;

    // This set will collect all locations of new
    // discovered directories, they are scanned in a
    // second run to avoid swapping between duplicated tracks
    mutable QMutex m_directoriesUnhashedMutex;
    QList<mixxx::FileAccess> m_directoriesUnhashed;

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
    int m_numRelocatedTracks;
};

typedef QSharedPointer<ScannerGlobal> ScannerGlobalPointer;
