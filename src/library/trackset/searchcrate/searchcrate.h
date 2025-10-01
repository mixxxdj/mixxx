#pragma once

#include "library/trackset/searchcrate/searchcrateid.h"
#include "util/db/dbnamedentity.h"

class SearchCrate : public DbNamedEntity<SearchCrateId> {
  public:
    explicit SearchCrate(SearchCrateId id = SearchCrateId())
            : DbNamedEntity(id),
              m_locked(false),
              m_autoDjSource(false) {
    }
    ~SearchCrate() override = default;

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
