#pragma once

#include "library/trackset/smarties/smartiesid.h"
#include "util/db/dbnamedentity.h"

class Smarties : public DbNamedEntity<SmartiesId> {
  public:
    explicit Smarties(SmartiesId id = SmartiesId())
            : DbNamedEntity(id),
              m_locked(false),
              m_autoDjSource(false) {
    }
    ~Smarties() override = default;

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
