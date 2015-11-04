// cuedao.h
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUEDAO_H
#define CUEDAO_H

#include <QMap>
#include <QSqlDatabase>

#include "trackinfoobject.h"
#include "library/dao/dao.h"

#define CUE_TABLE "cues"

class Cue;

class CueDAO : public DAO {
  public:
    CueDAO(QSqlDatabase& database);
    virtual ~CueDAO();
    void setDatabase(QSqlDatabase& database) { m_database = database; }

    void initialize();
    int cueCount();
    int numCuesForTrack(const int trackId);
    QList<Cue*> getCuesForTrack(const int trackId) const;
    bool deleteCuesForTrack(const int trackId);
    bool deleteCuesForTracks(const QList<int>& ids);
    bool saveCue(Cue* cue);
    bool deleteCue(Cue* cue);
    // TODO(XXX) once we refer to all tracks by their id and TIO has a getId()
    // method the first parameter here won't be necessary.
    void saveTrackCues(const int trackId, TrackInfoObject*);
  private:
    Cue* cueFromRow(const QSqlQuery& query) const;

    QSqlDatabase& m_database;
    mutable QMap<int, Cue*> m_cues;
};

#endif /* CUEDAO_H */
