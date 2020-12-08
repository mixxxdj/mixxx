#pragma once

#include <QObject>
#include <QHash>
#include <QString>
#include <QSqlDatabase>

#include "library/dao/dao.h"
#include "util/cache.h"

class LibraryHashDAO : public DAO {
  public:
    ~LibraryHashDAO() override = default;

    QHash<QString, mixxx::cache_key_t> getDirectoryHashes();
    mixxx::cache_key_t getDirectoryHash(const QString& dirPath);
    void saveDirectoryHash(const QString& dirPath, mixxx::cache_key_t hash);
    void updateDirectoryHash(const QString& dirPath, mixxx::cache_key_t newHash,
                             int dir_deleted);
    void markAsExisting(const QString& dirPath);
    void invalidateAllDirectories();
    void markUnverifiedDirectoriesAsDeleted();
    void removeDeletedDirectoryHashes();
    void updateDirectoryStatuses(const QStringList& dirPaths,
                                 const bool deleted, const bool verified);
    QStringList getDeletedDirectories();
};
