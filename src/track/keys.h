#pragma once

#include <grantlee/metatype.h>

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QPair>

#include "proto/keys.pb.h"

#define KEY_MAP_VERSION "KeyMap-1.0"

typedef QList<QPair<mixxx::track::io::key::ChromaticKey, double>> KeyChangeList;

class KeyFactory;

class Keys final {
    Q_GADGET
  public:
    Q_PROPERTY(QString traditional READ getTraditional)
    Q_PROPERTY(QString openkey READ getOpenkey)
    Q_PROPERTY(QString lancelot READ getLancelot)
    Q_PROPERTY(bool isValid READ isValid)

    explicit Keys(const QByteArray* pByteArray = nullptr);

    QString getTraditional() const;
    QString getOpenkey() const;
    QString getLancelot() const;

    // Serialization
    QByteArray toByteArray() const;

    // A string representing the version of the key-processing code that
    // produced this Keys instance. Used by KeysFactory for associating a given
    // serialization with the version that produced it.
    QString getVersion() const {
        return KEY_MAP_VERSION;
    }

    // A sub-version can be used to represent the preferences used to generate
    // the keys object.
    const QString& getSubVersion() const;
    void setSubVersion(const QString& subVersion);

    bool isValid() const;

    ////////////////////////////////////////////////////////////////////////////
    // Key calculations
    ////////////////////////////////////////////////////////////////////////////

    // Return the average key over the entire track if the key is valid.
    mixxx::track::io::key::ChromaticKey getGlobalKey() const;
    QString getGlobalKeyText() const;

  private:
    explicit Keys(const mixxx::track::io::key::KeyMap& m_keyMap);

    bool readByteArray(const QByteArray& byteArray);

    QString m_subVersion;
    mixxx::track::io::key::KeyMap m_keyMap;

    // For private constructor access.
    friend class KeyFactory;
};

bool operator==(const Keys& lhs, const Keys& rhs);

inline bool operator!=(const Keys& lhs, const Keys& rhs) {
    return !(lhs == rhs);
}

Q_DECLARE_METATYPE(Keys)

// FIXME(XXX) This should work according to grentlee docs, but it does not
GRANTLEE_BEGIN_LOOKUP(Keys)
if (property.isEmpty()) {
    return object.getOpenkey();
}
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP_PTR(Keys)
if (property.isEmpty()) {
    return object->getOpenkey();
}
GRANTLEE_END_LOOKUP
