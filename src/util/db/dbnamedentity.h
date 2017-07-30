#ifndef MIXXX_DBNAMEDENTITY_H
#define MIXXX_DBNAMEDENTITY_H


#include "util/db/dbentity.h"


// Base class for database entities with a non-empty name.
template<typename T> // where T is derived from DbId
class DbNamedEntity: public DbEntity<T> {
  public:
    ~DbNamedEntity() override = default;

    static QString normalizeName(const QString& name) {
        return name.trimmed();
    }
    bool hasName() const {
        DEBUG_ASSERT(normalizeName(m_name) == m_name); // already normalized
        return !m_name.isEmpty();
    }
    const QString& getName() const {
        return m_name;
    }
    void setName(const QString& name) {
        DEBUG_ASSERT(normalizeName(name) == name); // already normalized
        m_name = name;
    }
    void resetName() {
        m_name.clear();
        DEBUG_ASSERT(!hasName());
    }
    bool parseName(const QString& name) {
        QString normalizedName(normalizeName(name));
        if (name.isEmpty()) {
            return false;
        } else {
            setName(name);
            DEBUG_ASSERT(hasName());
            return true;
        }
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


#endif // MIXXX_DBNAMEDENTITY_H
