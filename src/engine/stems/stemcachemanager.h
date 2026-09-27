#pragma once

#include <QObject>
#include <QRecursiveMutex>
#include <QHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

class StemCacheManager : public QObject {
    Q_OBJECT
public:
    struct StemFiles {
        QString vocals;   // stem 0: vocals
        QString drums;    // stem 1: drums  
        QString bass;     // stem 2: bass
        QString other;    // stem 3: other
        QString stemFile; // native .stem.mp4 (5 streams + stem atom), if generated
        bool complete = false;
        QDateTime created;
    };

    using CacheKey = QByteArray; // SHA256 of track location + mtime + size

    static StemCacheManager& instance();

    // Check if stems are cached and ready
    bool hasStems(const CacheKey& key) const;

    // Get cached stem file paths (empty if not ready)
    StemFiles getStemFiles(const CacheKey& key) const;

    // Called when separation starts - returns false if already processing/done
    bool tryMarkProcessing(const CacheKey& key);

    // Called when separation completes successfully
    void markComplete(const CacheKey& key, const StemFiles& files);

    // Called when separation fails
    void markFailed(const CacheKey& key);

    // Generate cache key from track
    static CacheKey generateKey(const QString& trackLocation);

    // N18: mode-versioned cache key. Mode 4 (default) is identical to
    // generateKey() so all existing entries stay valid; mode 3 appends
    // "|mode=3" to the hashed material so 3-stem artifacts live in a
    // different dir and never poison 4-stem lookups.
    // N19: versioning intentionally unchanged after the default flip to
    // mode 3 (mode 4 stays legacy). Default mode-3 artifacts are versioned;
    // legacy mode-4 entries are never served as mode 3, and the 8-channel
    // reader layout is identical in both modes.
    static CacheKey generateKeyForMode(const QString& trackLocation, int stemMode);

    // Prune cache to max size (default 2GB)
    void pruneCache(qint64 maxSizeBytes = 2LL * 1024 * 1024 * 1024);

    // Directory where stems are stored.
    // Layout: <cacheDir>/<key>/<key>.stem.mp4, where key is the hex SHA256
    // of (path + mtime + size). cacheDir() is
    // ~/.local/share/mixxx/analysis/stems (AppDataLocation).
    static QString cacheDir();
    static QString stemDir(const CacheKey& key);
    static QString stemFilePath(const CacheKey& key);
    static QString indexPath();

signals:
    void separationStarted(const CacheKey& key);
    void separationFinished(const CacheKey& key, bool success);

private:
    StemCacheManager();
    ~StemCacheManager();

    void loadIndex();
    void saveIndex() const;

    mutable QRecursiveMutex m_mutex;
    QHash<CacheKey, StemFiles> m_index;
    QHash<CacheKey, bool> m_processing; // key -> in-progress
};

Q_DECLARE_METATYPE(StemCacheManager::StemFiles)