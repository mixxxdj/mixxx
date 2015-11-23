#ifndef DIRECTORYDAO_H
#define DIRECTORYDAO_H

#include <QSqlDatabase>
#include "library/dao/trackdao.h"

const QString DIRECTORYDAO_DIR = "directory";
const QString DIRECTORYDAO_TABLE = "directories";

enum ReturnCodes {
    SQL_ERROR,
    ALREADY_WATCHING,
    ALL_FINE
};

class DirectoryDAO : public DAO {
  public:

    DirectoryDAO(QSqlDatabase& database);
    virtual ~DirectoryDAO();

    void initialize();
    void setDatabase(QSqlDatabase& database) { m_database = database; }
    int addDirectory(const QString& dir);
    int removeDirectory(const QString& dir);
    QSet<TrackId> relocateDirectory(const QString& oldFolder, const QString& newFolder);
    QStringList getDirs();

  private:
    bool isChildDir(QString testDir, QString dirStr);
    QSqlDatabase& m_database;
};

#endif //DIRECTORYDAO_H
