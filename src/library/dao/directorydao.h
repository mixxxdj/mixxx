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
    void initialize(const QSqlDatabase& database) override {
        m_database = database;
    }

    int addDirectory(const QString& dir);
    int removeDirectory(const QString& dir);
    QList<RelocatedTrack> relocateDirectory(
            const QString& oldFolder,
            const QString& newFolder);
    QStringList getDirs();

  private:
    bool isChildDir(QString testDir, QString dirStr);
    QSqlDatabase m_database;
};

#endif //DIRECTORYDAO_H
