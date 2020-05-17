#pragma once

#include <QMap>
#include <QSqlDatabase>

#include "track/track.h"
#include "library/dao/dao.h"

#define CUE_TABLE "cues"

class Cue;

class CueDAO : public DAO {
  public:
    ~CueDAO() override = default;

    void initialize(const QSqlDatabase& database) override {
        m_database = database;
    }

    QList<CuePointer> getCuesForTrack(TrackId trackId) const;
    bool deleteCuesForTrack(TrackId trackId);
    bool deleteCuesForTracks(const QList<TrackId>& trackIds);
    bool saveCue(Cue* cue);
    bool deleteCue(Cue* cue);
    // TODO(XXX) once we refer to all tracks by their id and TIO has a getId()
    // method the first parameter here won't be necessary.
    void saveTrackCues(TrackId trackId, const QList<CuePointer>& cueList);
  private:
    CuePointer cueFromRow(const QSqlQuery& query) const;

    QSqlDatabase m_database;
    mutable QMap<int, CuePointer> m_cues;
};
