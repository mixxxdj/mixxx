#include "stemcachemanager.h"
#include "moc_stemcachemanager.cpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QThreadPool>

#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("StemCacheManager");
constexpr char kIndexFileName[] = "stem_index.json";
} // namespace

StemCacheManager& StemCacheManager::instance() {
    static StemCacheManager inst;
    return inst;
}

StemCacheManager::StemCacheManager() {
    qRegisterMetaType<StemFiles>("StemCacheManager::StemFiles");
    loadIndex();
    // pruneCache(); // defer to first use
}

StemCacheManager::~StemCacheManager() {
    saveIndex();
}

QString StemCacheManager::cacheDir() {
    // S5: stable analysis location (survives cache clears):
    // ~/.local/share/mixxx/analysis/stems
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            "/analysis/stems";
}

QString StemCacheManager::stemDir(const CacheKey& key) {
    return cacheDir() + "/" + QString::fromUtf8(key);
}

QString StemCacheManager::stemFilePath(const CacheKey& key) {
    // Hash-named native stem file: .../{hash}/{hash}.stem.mp4
    const QString hex = QString::fromUtf8(key);
    return stemDir(key) + "/" + hex + ".stem.mp4";
}

QString StemCacheManager::indexPath() {
    return cacheDir() + "/" + kIndexFileName;
}

StemCacheManager::CacheKey StemCacheManager::generateKey(const QString& trackLocation) {
    QFileInfo fi(trackLocation);
    QByteArray data = trackLocation.toUtf8();
    data.append(fi.lastModified().toString(Qt::ISODate).toUtf8());
    data.append(QByteArray::number(fi.size()));
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
}

StemCacheManager::CacheKey StemCacheManager::generateKeyForMode(
        const QString& trackLocation, int stemMode) {
    if (stemMode == 3) {
        QFileInfo fi(trackLocation);
        QByteArray data = trackLocation.toUtf8();
        data.append(fi.lastModified().toString(Qt::ISODate).toUtf8());
        data.append(QByteArray::number(fi.size()));
        data.append("|mode=3");
        return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
    }
    return generateKey(trackLocation);
}

void StemCacheManager::loadIndex() {
    QFile file(indexPath());
    if (!file.exists()) {
        QDir().mkpath(cacheDir());
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        kLogger.warning() << "Cannot open stem index:" << file.fileName();
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject root = doc.object();
    QJsonArray entries = root["entries"].toArray();

    QMutexLocker lock(&m_mutex);
    for (const QJsonValue& val : entries) {
        QJsonObject obj = val.toObject();
        CacheKey key = obj["key"].toString().toUtf8();
        StemFiles files;
        files.vocals = obj["vocals"].toString();
        files.drums = obj["drums"].toString();
        files.bass = obj["bass"].toString();
        files.other = obj["other"].toString();
        files.stemFile = obj["stemFile"].toString();
        files.complete = obj["complete"].toBool();
        files.created = QDateTime::fromString(obj["created"].toString(), Qt::ISODate);
        // Backward compat: pre-S5 entries stored track.stem.mp4 inside the
        // per-key dir. Accept them if the file still exists.
        if (files.stemFile.isEmpty()) {
            const QString legacy = stemDir(key) + "/track.stem.mp4";
            if (QFile::exists(legacy)) {
                files.stemFile = legacy;
            }
        }
        // Verify files still exist. Accept entries that have either the 4
        // WAVs (legacy/offline path) or a native .stem.mp4 (S5 analyzer
        // path writes only the .stem.mp4).
        const bool wavsOk = !files.vocals.isEmpty() &&
                QFile::exists(files.vocals) &&
                QFile::exists(files.drums) &&
                QFile::exists(files.bass) &&
                QFile::exists(files.other);
        const bool stemOk = !files.stemFile.isEmpty() &&
                QFile::exists(files.stemFile);
        if (files.complete && (wavsOk || stemOk)) {
            m_index[key] = files;
        }
    }
    kLogger.info() << "Loaded stem cache index:" << m_index.size() << "entries";
}

void StemCacheManager::saveIndex() const {
    QMutexLocker lock(&m_mutex);
    QJsonObject root;
    QJsonArray entries;
    for (auto it = m_index.constBegin(); it != m_index.constEnd(); ++it) {
        QJsonObject obj;
        obj["key"] = QString(it.key());
        obj["vocals"] = it.value().vocals;
        obj["drums"] = it.value().drums;
        obj["bass"] = it.value().bass;
        obj["other"] = it.value().other;
        obj["stemFile"] = it.value().stemFile;
        obj["complete"] = it.value().complete;
        obj["created"] = it.value().created.toString(Qt::ISODate);
        entries.append(obj);
    }
    root["entries"] = entries;
    root["version"] = 1;

    QDir().mkpath(cacheDir());
    QFile file(indexPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
    }
}

bool StemCacheManager::hasStems(const CacheKey& key) const {
    QMutexLocker lock(&m_mutex);
    auto it = m_index.find(key);
    return it != m_index.end() && it.value().complete;
}

StemCacheManager::StemFiles StemCacheManager::getStemFiles(const CacheKey& key) const {
    QMutexLocker lock(&m_mutex);
    auto it = m_index.find(key);
    return it != m_index.end() ? it.value() : StemFiles{};
}

bool StemCacheManager::tryMarkProcessing(const CacheKey& key) {
    QMutexLocker lock(&m_mutex);
    if (m_processing.contains(key)) return false;
    if (m_index.contains(key) && m_index[key].complete) return false;
    
    m_processing[key] = true;
    emit separationStarted(key);
    return true;
}

void StemCacheManager::markComplete(const CacheKey& key, const StemFiles& files) {
    QMutexLocker lock(&m_mutex);
    m_processing.remove(key);
    m_index[key] = files;
    saveIndex();
    emit separationFinished(key, true);
}

void StemCacheManager::markFailed(const CacheKey& key) {
    QMutexLocker lock(&m_mutex);
    m_processing.remove(key);
    emit separationFinished(key, false);
}

void StemCacheManager::pruneCache(qint64 maxSizeBytes) {
    QMutexLocker lock(&m_mutex);
    
    qint64 totalSize = 0;
    QList<std::pair<CacheKey, QDateTime>> lru;
    
    for (auto it = m_index.constBegin(); it != m_index.constEnd(); ++it) {
        if (!it.value().complete) continue;
        qint64 entrySize = 0;
        for (const QString& path : {it.value().vocals,
                     it.value().drums,
                     it.value().bass,
                     it.value().other,
                     it.value().stemFile}) {
            if (!path.isEmpty()) {
                QFileInfo fi(path);
                if (fi.exists()) {
                    entrySize += fi.size();
                }
            }
        }
        if (entrySize > 0) {
            totalSize += entrySize;
            lru.append({it.key(), it.value().created});
        }
    }

    if (totalSize <= maxSizeBytes) return;

    std::sort(lru.begin(), lru.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    for (const auto& [key, _] : lru) {
        if (totalSize <= maxSizeBytes) break;
        
        QString dir = stemDir(key);
        QDir d(dir);
        if (d.exists()) {
            qint64 removed = 0;
            for (const QFileInfo& fi : d.entryInfoList(QDir::Files)) {
                removed += fi.size();
                fi.dir().remove(fi.fileName());
            }
            totalSize -= removed;
        }
        m_index.remove(key);
    }
    saveIndex();
    kLogger.info() << "Pruned stem cache, remaining size:" << totalSize;
}