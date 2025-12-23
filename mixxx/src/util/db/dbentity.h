#pragma once

#include "util/db/dbid.h"


// Base class for database entities that are identified
// by a unique DbId.
template<typename T> // where T is derived from DbId
class DbEntity {
  public:
    virtual ~DbEntity() = default;

    T getId() const {
        return m_id;
    }
    bool hasId() const {
        return m_id.isValid();
    }
    void setId(T id) {
        m_id = id;
    }

  protected:
    DbEntity() = default;
    explicit DbEntity(T id)
        : m_id(id) {
    }

  private:
    T m_id;
};
