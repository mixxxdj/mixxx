#pragma once

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

    void saveTrackCues(TrackId trackId, const QList<CuePointer>& cueList) const;
    bool deleteCuesForTrack(TrackId trackId) const;
    bool deleteCuesForTracks(const QList<TrackId>& trackIds) const;

  private:
    bool saveCue(Cue* pCue) const;
    bool deleteCue(Cue* pCue) const;

    QSqlDatabase m_database;
};
