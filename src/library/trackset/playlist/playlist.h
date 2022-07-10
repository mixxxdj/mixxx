#pragma once

#include "library/trackset/playlist/playlistid.h"
#include "util/db/dbnamedentity.h"

class Playlist : public DbNamedEntity<PlaylistId> {
  public:
    explicit Playlist(PlaylistId id = PlaylistId())
            : DbNamedEntity(id) {
    }
    ~Playlist() override = default;
};
