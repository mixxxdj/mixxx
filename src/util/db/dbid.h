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

    /// Create an invalid instance
    constexpr DbId()
            : m_value(kInvalidValue) {
    }
    /// Try to convert from a QVariant
    ///
    /// If the conversion fails an invalid instance as fallback.
    explicit DbId(QVariant variant)
            : DbId(NoAssert{}, valueOf(std::move(variant))) {
    }
    /// Explicit checked conversion from inner value
    ///
    /// Creates an instance from the provided inner value. The value
    /// might be either valid or invalid. Triggers a debug assertion
    /// if an invalid value doesn't equal the predefined constant.
    explicit DbId(value_type value)
            : DbId(NoAssert{}, value) {
        DEBUG_ASSERT(isValid() || m_value == kInvalidValue);
    }

    /// Explicit unchecked conversion from inner value
    ///
    /// Creates an instance with the provided inner value without
    /// any consistency checks that could trigger an assertion.
    ///
    /// Use consciously, e.g. for testing.
    static constexpr DbId fromValueUnchecked(value_type value) {
        return DbId(NoAssert{}, value);
    }

    constexpr bool isValid() const {
        return isValidValue(m_value);
    }

    /// Explicit unchecked conversion into the inner value
    ///
    /// Returns the inner value, either valid or invalid.
    constexpr value_type valueMaybeInvalid() const {
        return m_value;
    }

    /// Explicit checked conversion into the inner value
    ///
    /// Returns the inner value. Triggers a debug assertion when
    /// invoked on invalid instances.
    value_type value() const {
        DEBUG_ASSERT(isValid());
        return m_value;
    }

    /// Implicit checked conversion of the inner value
    ///
    /// Returns the inner value. Triggers a debug assertion when
    /// invoked on invalid instances.
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
    /// Type tag for overloaded constructor that doesn't assert
    struct NoAssert {};
    constexpr DbId(NoAssert, value_type value)
            : m_value(value) {
    }

    static value_type valueOf(QVariant variant);

    value_type m_value;
};

inline constexpr bool operator==(const DbId& lhs, const DbId& rhs) {
    return lhs.valueMaybeInvalid() == rhs.valueMaybeInvalid();
}

inline constexpr bool operator!=(const DbId& lhs, const DbId& rhs) {
    return lhs.valueMaybeInvalid() != rhs.valueMaybeInvalid();
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
    return os << dbId.valueMaybeInvalid();
}

inline QDebug operator<<(QDebug debug, const DbId& dbId) {
    return debug << dbId.valueMaybeInvalid();
}

inline uint qHash(
        const DbId& dbId,
        uint seed = 0) {
    return qHash(dbId.valueMaybeInvalid(), seed);
}

Q_DECLARE_TYPEINFO(DbId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(DbId)
