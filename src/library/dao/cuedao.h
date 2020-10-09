#pragma once

#include <QSqlDatabase>

#include "library/dao/dao.h"
#include "track/cue.h"
#include "track/trackid.h"

#define CUE_TABLE "cues"

class Cue;

class CueDAO : public DAO {
  public:
    ~CueDAO() override = default;

    QList<CuePointer> getCuesForTrack(TrackId trackId) const;

    void saveTrackCues(TrackId trackId, const QList<CuePointer>& cueList) const;
    bool deleteCuesForTrack(TrackId trackId) const;
    bool deleteCuesForTracks(const QList<TrackId>& trackIds) const;

  private:
    bool saveCue(Cue* pCue) const;
    bool deleteCue(Cue* pCue) const;
};
