#ifndef KEYS_H
#define KEYS_H

#include <QByteArray>
#include <QMutex>
#include <QPair>
#include <QVector>

#include "proto/keys.pb.h"
#include "util/memory.h"

#define KEY_MAP_VERSION "KeyMap-1.0"

typedef QVector<QPair<mixxx::track::io::key::ChromaticKey, double> > KeyChangeList;

class KeyFactory;

class Keys {
  public:
    Keys(const Keys& other);
    explicit Keys(const QByteArray* pByteArray = nullptr);
    virtual ~Keys();

    Keys& operator=(const Keys& other);

    // Serialization
    virtual std::unique_ptr<QByteArray> toByteArray() const;

    // A string representing the version of the key-processing code that
    // produced this Keys instance. Used by KeysFactory for associating a given
    // serialization with the version that produced it.
    virtual QString getVersion() const;

    // A sub-version can be used to represent the preferences used to generate
    // the keys object.
    virtual QString getSubVersion() const;
    virtual void setSubVersion(const QString& subVersion);

    bool isValid() const;

    ////////////////////////////////////////////////////////////////////////////
    // Key calculations
    ////////////////////////////////////////////////////////////////////////////

    // Return the average key over the entire track if the key is valid.
    virtual mixxx::track::io::key::ChromaticKey getGlobalKey() const;
    virtual QString getGlobalKeyText() const;

  private:
    explicit Keys(const mixxx::track::io::key::KeyMap& m_keyMap);

    bool readByteArray(const QByteArray& byteArray);

    mutable QMutex m_mutex;

    QString m_subVersion;
    mixxx::track::io::key::KeyMap m_keyMap;

    // For private constructor access.
    friend class KeyFactory;
};

#endif /* KEYS_H */
