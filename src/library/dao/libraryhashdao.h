#ifndef LIBRARYHASHDAO_H
#define LIBRARYHASHDAO_H

#include <QObject>
#include <QSqlDatabase>
#include "library/dao/dao.h"

class LibraryHashDAO : public DAO {
  public:
    LibraryHashDAO(QSqlDatabase& database);
    virtual ~LibraryHashDAO();
    void setDatabase(QSqlDatabase& database) { m_database = database; };

    void initialize();
    int getDirectoryHash(const QString& dirPath);
    void saveDirectoryHash(const QString& dirPath, const int hash);
    void updateDirectoryHash(const QString& dirPath, const int newHash, const int dir_deleted);
    void markAsExisting(const QString& dirPath);
    void markAsVerified(const QString& dirPath);
    //void markAllDirectoriesAsDeleted();
    void invalidateAllDirectories();
    void markUnverifiedDirectoriesAsDeleted();
    void removeDeletedDirectoryHashes();
    void updateDirectoryStatuses(QStringList dirPaths, const bool deleted, const bool verified);
  private:
    QSqlDatabase &m_database;

};

#endif //LIBRARYHASHDAO_H
