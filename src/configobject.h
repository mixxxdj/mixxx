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

#include "util/debug.h"


// Class for the key for a specific configuration element. A key consists of a
// group and an item.

class ConfigKey {
  public:
    ConfigKey();
    ConfigKey(const QString& g, const QString& i);
    ConfigKey(const char* g, const char* i);
    static ConfigKey parseCommaSeparated(QString key);

    inline bool isNull() const {
        return group.isNull() && item.isNull();
    }

    QString group, item;
};
Q_DECLARE_METATYPE(ConfigKey);

// comparison function for ConfigKeys. Used by a QHash in ControlObject
inline bool operator==(const ConfigKey& c1, const ConfigKey& c2) {
    return c1.group == c2.group && c1.item == c2.item;
}

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
    ConfigValue(QString _value);
    ConfigValue(int _value);
    inline ConfigValue(QDomNode /* node */) {
        reportFatalErrorAndQuit("ConfigValue from QDomNode not implemented here");
    }
    void valCopy(const ConfigValue& _value);

    QString value;
    friend bool operator==(const ConfigValue& s1, const ConfigValue& s2);
};

class ConfigValueKbd : public ConfigValue {
  public:
    ConfigValueKbd();
    ConfigValueKbd(QString _value);
    ConfigValueKbd(QKeySequence key);
    inline ConfigValueKbd(QDomNode /* node */) {
        reportFatalErrorAndQuit("ConfigValueKbd from QDomNode not implemented here");
    }
    void valCopy(const ConfigValueKbd& v);
    friend bool operator==(const ConfigValueKbd& s1, const ConfigValueKbd& s2);

    QKeySequence m_qKey;
};

template <class ValueType> class ConfigOption {
  public:
    ConfigOption() { val = NULL; key = NULL;};
    ConfigOption(ConfigKey* _key, ValueType* _val) { key = _key ; val = _val; };
    virtual ~ConfigOption() {
        delete key;
        delete val;
    }
    ValueType* val;
    ConfigKey* key;
};

template <class ValueType> class ConfigObject {
  public:
    ConfigKey key;
    ValueType value;
    ConfigOption<ValueType> option;

    ConfigObject(QString file);
    ConfigObject(QDomNode node);
    ~ConfigObject();
    ConfigOption<ValueType> *set(ConfigKey, ValueType);
    ConfigOption<ValueType> *get(ConfigKey key);
    bool exists(ConfigKey key);
    ConfigKey *get(ValueType v);
    QString getValueString(ConfigKey k);
    QString getValueString(ConfigKey k, const QString& default_string);
    QHash<ConfigKey, ValueType> toHash() const;

    void clear();
    void reopen(QString file);
    void Save();

    // Returns the resource path -- the path where controller presets, skins,
    // library schema, keyboard mappings, and more are stored.
    QString getResourcePath() const;

    // Returns the settings path -- the path where user data (config file,
    // library SQLite database, etc.) is stored.
    QString getSettingsPath() const;

  protected:
    QList<ConfigOption<ValueType>*> m_list;
    QString m_filename;

    // Loads and parses the configuration file. Returns false if the file could
    // not be opened; otherwise true.
    bool Parse();
};

#endif
