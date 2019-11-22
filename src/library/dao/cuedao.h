// cuedao.h
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUEDAO_H
#define CUEDAO_H

#include <QMap>
#include <QSqlDatabase>

#include "track/track.h"
#include "library/dao/dao.h"
#include "preferences/hotcuecolorpalettesettings.h"

#define CUE_TABLE "cues"

class Cue;

class CueDAO : public DAO {
  public:
    explicit CueDAO(const UserSettingsPointer& pConfig);
    ~CueDAO() override = default;

    void initialize(const QSqlDatabase& database) override {
        m_database = database;
    }

    int cueCount();
    int numCuesForTrack(TrackId trackId);
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
    UserSettingsPointer m_pConfig;
    HotcueColorPaletteSettings m_colorPaletteSettings;
};

#endif /* CUEDAO_H */
