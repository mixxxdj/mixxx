#pragma once

#include <QObject>

#include "library/trackset/playlist/playlistid.h"
#include "preferences/usersettings.h"

class TrackCollection;
class Playlist;

class PlaylistFeatureHelper : public QObject {
    Q_OBJECT

  public:
    PlaylistFeatureHelper(
            TrackCollection* pTrackCollection,
            UserSettingsPointer pConfig);
    ~PlaylistFeatureHelper() override = default;

    PlaylistId createEmptyPlaylist();
    PlaylistId duplicatePlaylist(const Playlist& oldPlaylist);

  private:
    QString proposeNameForNewPlaylist(
            const QString& initialName = QString()) const;

    TrackCollection* m_pTrackCollection;

    UserSettingsPointer m_pConfig;
};
