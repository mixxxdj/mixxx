// cuedao.h
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUEDAO_H
#define CUEDAO_H

#include <QMap>
#include <QSqlDatabase>

#include "library/dao/dao.h"

#define CUE_TABLE "cues"

class Cue;

class CueDAO : public DAO {
  public:
    CueDAO(QSqlDatabase& database);
    virtual ~CueDAO();

    void initialize();
    int cueCount();
    int numCuesForTrack(int trackId);
    Cue* getCue(int cueId);
    QList<Cue*> getCuesForTrack(int trackId);
    bool deleteCuesForTrack(int trackId);
    bool saveCue(Cue* cue);
    bool deleteCue(Cue* cue);
  private:
    Cue* cueFromRow(QSqlQuery& query);

    QSqlDatabase& m_database;
    QMap<int, Cue*> m_cues;
};

#endif /* CUEDAO_H */
