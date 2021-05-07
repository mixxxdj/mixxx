#pragma once

#include <QtDebug>

#include "aoide/json/json.h"

namespace aoide {

namespace json {

class EntityUid final {
  public:
    typedef QString value_t;

    explicit EntityUid(
            value_t value = value_t())
            : m_value(value) {
    }

    bool isValid() const {
        return !m_value.isEmpty();
    }

    static std::optional<EntityUid> fromQJsonValue(
            const QJsonValue& value);

    const value_t& value() const {
        return m_value;
    }
    operator const value_t&() const {
        return m_value;
    }
    operator QJsonValue() const {
        return QJsonValue{m_value};
    }

  private:
    value_t m_value;
};

inline bool operator==(const EntityUid& lhs, const EntityUid& rhs) {
    return lhs.value() == rhs.value();
}

inline bool operator!=(const EntityUid& lhs, const EntityUid& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const EntityUid& arg) {
    return dbg << arg.value();
}

class EntityRevision final {
  public:
    typedef quint64 value_t;

    static constexpr value_t kInvalidValue = 0;

    explicit EntityRevision(
            value_t value = kInvalidValue)
            : m_value(value) {
    }

    bool isValid() const {
        return m_value != kInvalidValue;
    }

    static std::optional<EntityRevision> fromQJsonValue(
            const QJsonValue& value);

    value_t value() const {
        return m_value;
    }
    operator value_t() const {
        return m_value;
    }
    operator QJsonValue() const {
        return QJsonValue{static_cast<qint64>(m_value)};
    }

  private:
    quint64 m_value;
};

inline bool operator==(const EntityRevision& lhs, const EntityRevision& rhs) {
    return lhs.value() == rhs.value();
}

inline bool operator!=(const EntityRevision& lhs, const EntityRevision& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const EntityRevision& arg) {
    return dbg << arg.value();
}

class EntityHeader : public Array {
  public:
    explicit EntityHeader(
            QJsonArray jsonArray = QJsonArray());
    ~EntityHeader() override = default;

    EntityUid uid() const;

    EntityRevision rev() const;
};

} // namespace json

} // namespace aoide

Q_DECLARE_METATYPE(aoide::json::EntityUid);
Q_DECLARE_METATYPE(aoide::json::EntityRevision);
Q_DECLARE_METATYPE(aoide::json::EntityHeader);
