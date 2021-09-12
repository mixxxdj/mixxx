#pragma once

#include <QString>
#include <QVariant>
#include <QtDebug>
#include <functional>
#include <ostream>
#include <utility>

#include "util/assert.h"

/// Base class for ID values of objects that are stored in the database.
//
/// Implicit conversion from/to the native value type has been disabled
/// on purpose in favor of explicit conversion. The internal value type
/// is also hidden as an implementation detail.
//
/// Although used as a base class this class has a non-virtual destructor,
/// because destruction is trivial and for maximum efficiency! Derived
/// classes may add additional behavior (= member functions), but should
/// not add any additional state (= member variables). Inheritance is
/// only needed for type-safety.
class DbId {
  public:
    /// Alias for the corresponding native type. It keeps the
    /// implementation of this class flexible if we ever gonna
    /// need to change it from 'int' to 'long' or any other type.
    typedef int value_type;

    static constexpr value_type kInvalidValue = -1;
    static constexpr value_type kMinValue = 0;

    static constexpr bool isValidValue(value_type value) {
        static_assert(kInvalidValue < kMinValue);
        return value >= kMinValue;
    }

    /// Type tag for overloaded constructor that doesn't assert
    struct NoAssert {};

    constexpr DbId()
            : m_value(kInvalidValue) {
    }
    constexpr DbId(NoAssert, value_type value)
            : m_value(value) {
    }
    explicit DbId(value_type value)
            : DbId(NoAssert{}, value) {
        DEBUG_ASSERT(isValid() || m_value == kInvalidValue);
    }
    explicit DbId(QVariant variant)
            : DbId(NoAssert{}, valueOf(std::move(variant))) {
    }

    constexpr bool isValid() const {
        return isValidValue(m_value);
    }

    constexpr value_type valueNoAssert() const {
        return m_value;
    }
    value_type value() const {
        DEBUG_ASSERT(isValid());
        return m_value;
    }
    operator value_type() const {
        return value();
    }

    std::size_t hash() const {
        return std::hash<value_type>()(m_value);
    }

    typedef std::function<std::size_t(const DbId& dbid)> hash_fun_t;
    static std::size_t hash_fun(const DbId& dbid) {
        return dbid.hash();
    }

    /// Convert into QVariant
    ///
    /// This function is used for value binding in DB queries with bindValue().
    QVariant toVariant() const {
        return isValid() ? QVariant{m_value} : QVariant{};
    }

    /// Convert into QString
    ///
    /// This function is used for formatting comma-separated lists of ids in DB queries.
    QString toString() const {
        return isValid() ? QString::number(m_value) : QString{};
    }

  private:
    static value_type valueOf(QVariant variant);

    value_type m_value;
};

inline constexpr bool operator==(const DbId& lhs, const DbId& rhs) {
    return lhs.valueNoAssert() == rhs.valueNoAssert();
}

inline constexpr bool operator!=(const DbId& lhs, const DbId& rhs) {
    return lhs.valueNoAssert() != rhs.valueNoAssert();
}

inline bool operator<(const DbId& lhs, const DbId& rhs) {
    return lhs.value() < rhs.value();
}

inline bool operator>(const DbId& lhs, const DbId& rhs) {
    return lhs.value() > rhs.value();
}

inline bool operator<=(const DbId& lhs, const DbId& rhs) {
    return lhs.value() <= rhs.value();
}

inline bool operator>=(const DbId& lhs, const DbId& rhs) {
    return lhs.value() >= rhs.value();
}

inline std::ostream& operator<<(std::ostream& os, const DbId& dbId) {
    return os << dbId.valueNoAssert();
}

inline QDebug operator<<(QDebug debug, const DbId& dbId) {
    return debug << dbId.valueNoAssert();
}

inline uint qHash(
        const DbId& dbId,
        uint seed = 0) {
    return qHash(dbId.valueNoAssert(), seed);
}

Q_DECLARE_TYPEINFO(DbId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(DbId)
