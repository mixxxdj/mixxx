/***************************************************************************
                          configobject.h  -  description
                             -------------------
    begin                : Thu Jun 6 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGOBJECT_H
#define CONFIGOBJECT_H

#include <QString>
#include <QFile>
#include <QKeySequence>
#include <QDomNode>
#include <QMap>
#include <QHash>
#include <QMetaType>
#include <QReadWriteLock>

#include "util/debug.h"


// Class for the key for a specific configuration element. A key consists of a
// group and an item.

class ConfigKey {
  public:
    ConfigKey(); // is required for qMetaTypeConstructHelper()
    ConfigKey(const ConfigKey& key);
    ConfigKey(const QString& g, const QString& i);
    static ConfigKey parseCommaSeparated(const QString& key);

    inline bool isEmpty() const {
        return group.isEmpty() && item.isEmpty();
    }

    inline bool isNull() const {
        return group.isNull() && item.isNull();
    }

    // comparison function for ConfigKeys. Used by a QHash in ControlObject
    friend inline bool operator==(const ConfigKey& lhs, const ConfigKey& rhs) {
        return lhs.group == rhs.group && lhs.item == rhs.item;
    }

    // comparison function for ConfigKeys. Used by a QMap in ControlObject
    friend inline bool operator<(const ConfigKey& lhs, const ConfigKey& rhs) {
        int groupResult = lhs.group.compare(rhs.group);
        if (groupResult == 0) {
            return lhs.item < rhs.item;
        }
        return (groupResult < 0);
    }
    QString group, item;
};
Q_DECLARE_METATYPE(ConfigKey);


// stream operator function for trivial qDebug()ing of ConfigKeys
inline QDebug operator<<(QDebug stream, const ConfigKey& c1) {
    stream << c1.group << "," << c1.item;
    return stream;
}

// QHash hash function for ConfigKey objects.
inline uint qHash(const ConfigKey& key) {
    return qHash(key.group) ^ qHash(key.item);
}

inline uint qHash(const QKeySequence& key) {
    return qHash(key.toString());
}


// The value corresponding to a key. The basic value is a string, but can be
// subclassed to more specific needs.
class ConfigValue {
  public:
    ConfigValue();
    ConfigValue(const QString& value);
    ConfigValue(int value);
    inline ConfigValue(const QDomNode& /* node */) {
        reportFatalErrorAndQuit("ConfigValue from QDomNode not implemented here");
    }
    void valCopy(const ConfigValue& value);

    QString value;
    friend bool operator==(const ConfigValue& s1, const ConfigValue& s2);
};

inline uint qHash(const ConfigValue& key) {
    return qHash(key.value.toUpper());
}

class ConfigValueKbd : public ConfigValue {
  public:
    ConfigValueKbd();
    ConfigValueKbd(const QString& _value);
    ConfigValueKbd(const QKeySequence& key);
    inline ConfigValueKbd(const QDomNode& /* node */) {
        reportFatalErrorAndQuit("ConfigValueKbd from QDomNode not implemented here");
    }
    void valCopy(const ConfigValueKbd& v);
    friend bool operator==(const ConfigValueKbd& s1, const ConfigValueKbd& s2);

    QKeySequence m_qKey;
};

template <class ValueType> class ConfigObject {
  public:
    ConfigObject(const QString& file);
    ConfigObject(const QDomNode& node);
    ~ConfigObject();

    void set(const ConfigKey& k, const ValueType& v);
    ValueType get(const ConfigKey& k) const;
    bool exists(const ConfigKey& key) const;
    QString getValueString(const ConfigKey& k) const;
    QString getValueString(const ConfigKey& k, const QString& default_string) const;
    QMultiHash<ValueType, ConfigKey> transpose() const;

    void reopen(const QString& file);
    void save();

    // Returns the resource path -- the path where controller presets, skins,
    // library schema, keyboard mappings, and more are stored.
    QString getResourcePath() const {
        return m_resourcePath;
    }

    // Returns the settings path -- the path where user data (config file,
    // library SQLite database, etc.) is stored.
    QString getSettingsPath() const {
        return m_settingsPath;
    }

  protected:
    // We use QMap because we want a sorted list in mixxx.cfg
    QMap<ConfigKey, ValueType> m_values;
    mutable QReadWriteLock m_valuesLock;
    QString m_filename;
    const QString m_resourcePath;
    const QString m_settingsPath;

    // Loads and parses the configuration file. Returns false if the file could
    // not be opened; otherwise true.
    bool parse();
};

#endif
