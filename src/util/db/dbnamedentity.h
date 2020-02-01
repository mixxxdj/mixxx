#pragma once

#include "util/db/dbentity.h"


inline QString parseEntityName(const QString& name) {
    return name.trimmed();
}
inline bool isValidEntityName(const QString& name) {
    DEBUG_ASSERT(name == parseEntityName(name));
    return !name.isEmpty();
}

// Base class for database entities with a non-empty name.
template<typename T> // where T is derived from DbId
class DbNamedEntity: public DbEntity<T> {
  public:
    ~DbNamedEntity() override = default;

    bool hasName() const {
        return isValidEntityName(m_name);
    }
    const QString& getName() const {
        return m_name;
    }
    void setName(QString name) {
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
