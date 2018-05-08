#ifndef MIXXX_DBID_H
#define MIXXX_DBID_H


#include <ostream>
#include <utility>

#include <QString>
#include <QVariant>
#include <QtDebug>

#include "util/assert.h"


// Base class for ID values of objects that are stored in the database.
//
// Implicit conversion from/to the native value type has been disabled
// on purpose in favor of explicit conversion. The internal value type
// is also hidden as an implementation detail.
//
// Although used as a base class this class has a non-virtual destructor,
// because destruction is trivial and for maximum efficiency! Derived
// classes may add additional behavior (= member functions), but should
// not add any additional state (= member variables). Inheritance is
// only needed for type-safety.
class DbId {
protected:
    // Alias for the corresponding native type. This typedef
    // should actually not be needed by users of this class,
    // but it keeps the implementation of this class flexible
    // if we ever gonna need to change it from 'int' to 'long'
    // or any other type.
    typedef int value_type;

public:
    DbId()
        : m_value(kInvalidValue) {
        DEBUG_ASSERT(!isValid());
    }
    explicit DbId(value_type value)
        : m_value(value) {
        DEBUG_ASSERT(isValid() || (kInvalidValue == m_value));
    }
    explicit DbId(QVariant variant)
        : DbId(valueOf(std::move(variant))) {
    }

    bool isValid() const {
        return isValidValue(m_value);
    }

    // This function is needed for backward compatibility and
    // should only be used within legacy code. It can be deleted
    // after all integer IDs have been replaced by their type-safe
    // counterparts.
    int toInt() const {
        return m_value;
    }

    // This function should be used for value binding in DB queries
    // with bindValue().
    QVariant toVariant() const {
        return QVariant(m_value);
    }

    QString toString() const {
        return QString::number(m_value);
    }

    friend bool operator==(const DbId& lhs, const DbId& rhs) {
        return lhs.m_value == rhs.m_value;
    }

    friend bool operator!=(const DbId& lhs, const DbId& rhs) {
        return lhs.m_value != rhs.m_value;
    }

    friend bool operator<(const DbId& lhs, const DbId& rhs) {
        return lhs.m_value < rhs.m_value;
    }

    friend bool operator>(const DbId& lhs, const DbId& rhs) {
        return lhs.m_value > rhs.m_value;
    }

    friend bool operator<=(const DbId& lhs, const DbId& rhs) {
        return lhs.m_value <= rhs.m_value;
    }

    friend bool operator>=(const DbId& lhs, const DbId& rhs) {
        return lhs.m_value >= rhs.m_value;
    }

    friend std::ostream& operator<<(std::ostream& os, const DbId& dbId) {
        return os << dbId.m_value;
    }

    friend QDebug operator<<(QDebug debug, const DbId& dbId) {
        return debug << dbId.m_value;
    }

    friend uint qHash(const DbId& dbId) {
        return qHash(dbId.m_value);
    }

private:
    static const value_type kInvalidValue = -1;

    static const QVariant::Type kVariantType;

    static bool isValidValue(value_type value) {
        return 0 <= value;
    }

    static value_type valueOf(QVariant /*pass-by-value*/ variant);

    value_type m_value;
};

Q_DECLARE_TYPEINFO(DbId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(DbId)


#endif // MIXXX_DBID_H
