#pragma once

#include <QString>

#define PLAYLIST_TABLE "Playlists"
#define PLAYLIST_TRACKS_TABLE "PlaylistTracks"

const QString PLAYLISTTABLE_ID = "id";
const QString PLAYLISTTABLE_NAME = "name";

// TODO(XXX): Fix AutoDJ database design.
// Crates should have no dependency on AutoDJ stuff. Which
// playlists are used as a source for AutoDJ has to be stored
// and managed by the AutoDJ component in a separate table.
// This refactoring should be deferred until consensus on the
// redesign of the AutoDJ feature has been reached. The main
// ideas of the new design should be documented for verification
// before starting to code.
const QString PLAYLISTTABLE_AUTODJ_SOURCE = "autodj_source";

const QString PLAYLISTTRACKSTABLE_PLAYLISTID = "playlist_id";
const QString PLAYLISTTRACKSTABLE_TRACKID = "track_id";
