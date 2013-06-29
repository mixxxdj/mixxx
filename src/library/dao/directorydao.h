#ifndef DIRECTORYDAO_H
#define DIRECTORYDAO_H

#include <QSqlDatabase>
#include "library/dao/dao.h"

const QString DIRECTORYDAO_DIR = "directory";
const QString DIRECTORYDAO_TABLE = "directories";

class DirectoryDAO : public DAO {
  public:
    DirectoryDAO(QSqlDatabase& database);
    virtual ~DirectoryDAO();

    void initialize();
    void setDatabase(QSqlDatabase& database) { m_database = database; };
    bool addDirectory(QString dir);
    bool purgeDirectory(QString dir);
    bool relocateDirectory(QString oldFolder,QString newFolder);
    QStringList getDirs();

  private:
    QSqlDatabase m_database;
};

#endif //DIRECTORYDAO_H
