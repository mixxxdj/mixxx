#ifndef MIXXX_CRATE_H
#define MIXXX_CRATE_H


#include "library/crate/crateid.h"

#include "util/db/dbnamedentity.h"


class Crate: public DbNamedEntity<CrateId> {
public:
    explicit Crate(CrateId id = CrateId())
        : DbNamedEntity(id),
          m_locked(false),
          m_autoDjSource(false) {
    }
    ~Crate() override {}

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


#endif // MIXXX_CRATE_H
