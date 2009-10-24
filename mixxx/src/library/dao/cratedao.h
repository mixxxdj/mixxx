// cratedao.h
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CRATEDAO_H
#define CRATEDAO_H

#include <QSqlDatabase>

#include "library/dao/dao.h"

class CrateDAO : public virtual DAO {
  public:
    CrateDAO(QSqlDatabase& database);
    virtual ~CrateDAO();

    // Initialize this DAO, create the tables it relies on, etc.
    void initialize();

    unsigned int crateCount();
    bool createCrate(const QString& name);
    bool deleteCrate(int crateId);
    int getCrateIdByName(const QString& name);
    QString crateName(int crateId);
    unsigned int crateSize(int crateId);
    bool addTrackToCrate(int trackId, int crateId);
    bool removeTrackFromCrate(int trackId, int crateId);

  private:
    QSqlDatabase& m_database;
};

#endif /* CRATEDAO_H */

