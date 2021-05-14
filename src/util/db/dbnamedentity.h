#pragma once

#include "util/db/dbentity.h"

// Base class for database entities with a non-empty name.
template<typename T> // where T is derived from DbId
class DbNamedEntity: public DbEntity<T> {
  public:
    ~DbNamedEntity() override = default;

    bool hasName() const {
        return !m_name.isEmpty();
    }
    const QString& getName() const {
        return m_name;
    }
    void setName(QString name) {
        // Due to missing trimming names with only whitespaces
        // may occur in the database and can't we assert on
        // this here!
        DEBUG_ASSERT(!name.isEmpty());
        m_name = std::move(name);
    }
    void resetName() {
        m_name.clear();
        DEBUG_ASSERT(!hasName());
    }

  protected:
    DbNamedEntity() = default;
    explicit DbNamedEntity(T id)
        : DbEntity<T>(std::forward<T>(id)) {
    }

  private:
    QString m_name;
};

template<typename T>
QDebug operator<<(QDebug debug, const DbNamedEntity<T>& entity) {
    return debug << QString("%1 '%2'").arg(entity.getId().toString(), entity.getName());
}
