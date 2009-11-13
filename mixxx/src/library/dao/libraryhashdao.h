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
    void markAllDirectoriesAsDeleted();
  private:
    QSqlDatabase &m_database;
    
};

#endif //LIBRARYHASHDAO_H 
