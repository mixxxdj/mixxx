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
    int getDirectoryHash(QString dirPath);
    void saveDirectoryHash(QString dirPath, int hash);
    void updateDirectoryHash(QString dirPath, int newHash, int dir_deleted);
    void markAsExisting(QString dirPath);
    void markAsVerified(QString dirPath);
    //void markAllDirectoriesAsDeleted();
    void invalidateAllDirectories();
    void markUnverifiedDirectoriesAsDeleted();
    void removeDeletedDirectoryHashes();
    void updateDirectoryStatuses(QStringList dirPaths, bool deleted, bool verified);
  private:
    QSqlDatabase &m_database;

};

#endif //LIBRARYHASHDAO_H
