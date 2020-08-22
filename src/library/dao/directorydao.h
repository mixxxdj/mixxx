#pragma once

#include <QList>

#include "library/dao/dao.h"
#include "library/relocatedtrack.h"

const QString DIRECTORYDAO_DIR = "directory";
const QString DIRECTORYDAO_TABLE = "directories";

enum ReturnCodes {
    SQL_ERROR,
    ALREADY_WATCHING,
    ALL_FINE
};

class DirectoryDAO : public DAO {
  public:
    ~DirectoryDAO() override = default;

    void initialize(const QSqlDatabase& database) override {
        m_database = database;
    }

    QStringList getDirs() const;

    int addDirectory(const QString& dir) const;
    int removeDirectory(const QString& dir) const;

    QList<RelocatedTrack> relocateDirectory(
            const QString& oldFolder,
            const QString& newFolder) const;

  private:
    QSqlDatabase m_database;
};
