#pragma once

#include <QString>
#include <QVariant>
#include <QtDebug>
#include <functional>
#include <ostream>
#include <utility>

#include "util/assert.h"
#include "util/compatibility/qhash.h"

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
  public:
    constexpr DbId()
            : m_value(kInvalidValue) {
    }
    explicit DbId(const QVariant& variant)
            : m_value(valueOf(variant)) {
    }

    DbId(int) = delete;

    bool isValid() const {
        return isValidValue(m_value);
    }

    std::size_t hash() const {
        return std::hash<int>()(m_value);
    }

    typedef std::function<std::size_t (const DbId& dbid)> hash_fun_t;
    static std::size_t hash_fun(const DbId& dbid) {
        return dbid.hash();
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

    friend QDataStream& operator<<(QDataStream& out, const DbId& dbId) {
        // explicit cast as recommended by Qt docs
        return out << static_cast<quint32>(dbId.m_value);
    }

    friend QDataStream& operator>>(QDataStream& in, DbId& dbId) {
        quint32 v;
        in >> v;
        dbId.m_value = v;
        return in;
    }

    friend qhash_seed_t qHash(
            const DbId& dbId,
            qhash_seed_t seed = 0) {
        return qHash(dbId.m_value, seed);
    }

private:
  static constexpr int kInvalidValue = -1;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  static const QMetaType kVariantType;
#else
    static const QVariant::Type kVariantType;
#endif

  static bool isValidValue(int value) {
        return 0 <= value;
  }

  static int valueOf(const QVariant& variant) {
        bool ok;
        int value = variant.toInt(&ok);
        if (ok && isValidValue(value)) {
            return value;
        }
        qCritical() << "Invalid database identifier value:"
                    << variant;
        return kInvalidValue;
  }

  int m_value;
};

Q_DECLARE_TYPEINFO(DbId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(DbId)
