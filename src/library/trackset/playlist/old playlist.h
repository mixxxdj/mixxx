#pragma once

#include "library/trackset/playlist/playlistid.h"
#include "util/db/dbnamedentity.h"

class Playlist : public DbNamedEntity<PlaylistId> {
  public:
    explicit Playlist(PlaylistId id = PlaylistId())
            : DbNamedEntity(id),
              m_locked(false),
              m_autoDjSource(false) {
    }
    ~Playlist() override = default;

    bool isLocked() const {
        return m_locked;
    }
    void setLocked(bool locked = true) {
        m_locked = locked;
    }

    bool isAutoDjSource() const {
        return m_autoDjSource;
    }
    void setAutoDjSource(bool autoDjSource = true) {
        m_autoDjSource = autoDjSource;
    }

  private:
    bool m_locked;
    bool m_autoDjSource;
};
