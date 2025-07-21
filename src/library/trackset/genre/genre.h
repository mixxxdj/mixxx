#pragma once

#include "library/trackset/genre/genreid.h"
#include "util/db/dbnamedentity.h"

class Genre : public DbNamedEntity<GenreId> {
  public:
    explicit Genre(GenreId id = GenreId())
            : DbNamedEntity(id),
              m_locked(false),
              m_autoDjSource(false) {
    }
    ~Genre() override = default;

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
