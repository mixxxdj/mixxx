#ifndef DIRECTORYDAO_H
#define DIRECTORYDAO_H

#include <QSqlDatabase>
#include "library/dao/dao.h"

const QString DIRECTORYDAO_DIR = "directory";
const QString DIRECTORYDAO_ID  = "dir_id";
const QString DIRECTORYDAO_TABLE = "directories";

class DirectoryDAO : public DAO {
  public:
    // normal method
    DirectoryDAO(QSqlDatabase& database);
    DirectoryDAO(const DirectoryDAO& directoryDao);
    virtual ~DirectoryDAO();

    void initialize();
    bool addDirectory(QString dir);
    bool purgeDirectory(QString dir);
    bool relocateDirectory(QString oldFolder,QString newFolder);
    bool updateTrackLocations(QString dir);
    QList<int> getDirIds(QStringList& dirs);
    int getDirId(const QString dir);
    QStringList getDirs();

  private:
    QSqlDatabase m_database;
};

#endif //DIRECTORYDAO_H
