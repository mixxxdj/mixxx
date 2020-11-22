#ifndef PREFERENCES_CONFIGOBJECT_H
#define PREFERENCES_CONFIGOBJECT_H

#include <QString>
#include <QKeySequence>
#include <QDomNode>
#include <QMap>
#include <QHash>
#include <QMetaType>
#include <QReadWriteLock>

#include "util/assert.h"
#include "util/debug.h"

// Class for the key for a specific configuration element. A key consists of a
// group and an item.
class ConfigKey final {
  public:
    ConfigKey() = default; // is required for qMetaTypeConstructHelper()
    ConfigKey(QString group, QString item)
            : group(std::move(group)),
              item(std::move(item)) {
    }

    static ConfigKey parseCommaSeparated(const QString& key);

    bool isValid() const {
        return !group.isEmpty() && !item.isEmpty();
    }

    QString group;
    QString item;
};
Q_DECLARE_METATYPE(ConfigKey);

// comparison function for ConfigKeys. Used by a QHash in ControlObject
inline bool operator==(const ConfigKey& lhs, const ConfigKey& rhs) {
    return lhs.group == rhs.group &&
            lhs.item == rhs.item;
}

// comparison function for ConfigKeys. Used by a QHash in ControlObject
inline bool operator!=(const ConfigKey& lhs, const ConfigKey& rhs) {
    return !(lhs == rhs);
}

// comparison function for ConfigKeys. Used by a QMap in ControlObject
inline bool operator<(const ConfigKey& lhs, const ConfigKey& rhs) {
    int groupResult = lhs.group.compare(rhs.group);
    if (groupResult == 0) {
        return lhs.item < rhs.item;
    }
    return (groupResult < 0);
}

// stream operator function for trivial qDebug()ing of ConfigKeys
inline QDebug operator<<(QDebug stream, const ConfigKey& configKey) {
    stream << configKey.group << "," << configKey.item;
    return stream;
}

// QHash hash function for ConfigKey objects.
inline uint qHash(
        const ConfigKey& key,
        uint seed = 0) {
    return qHash(key.group, seed) ^
            qHash(key.item, seed);
}

// The value corresponding to a key. The basic value is a string, but can be
// subclassed to more specific needs.
class ConfigValue {
  public:
    ConfigValue() = default;
    virtual ~ConfigValue() = default;
    // Only allow non-explicit QString -> ConfigValue conversion for
    // convenience. All other types must be explicit.
    ConfigValue(QString value)
            : value(std::move(value)) {
    }
    explicit ConfigValue(int value);
    explicit ConfigValue(double value);
    explicit ConfigValue(const QDomNode&) {
        reportFatalErrorAndQuit("ConfigValue from QDomNode not implemented here");
    }
    bool isNull() const { return value.isNull(); }

    QString value;
};

inline bool operator==(const ConfigValue& lhs, const ConfigValue& rhs) {
    return lhs.value.toUpper() == rhs.value.toUpper();
}

inline bool operator!=(const ConfigValue& lhs, const ConfigValue& rhs) {
    return !(lhs == rhs);
}

inline uint qHash(
        const ConfigValue& key,
        uint seed = 0) {
    return qHash(key.value.toUpper(), seed);
}

class ConfigValueKbd : public ConfigValue {
  public:
    ConfigValueKbd() = default;
    ~ConfigValueKbd() override = default;
    explicit ConfigValueKbd(const QKeySequence& keys);
    explicit ConfigValueKbd(const QDomNode) {
        reportFatalErrorAndQuit("ConfigValueKbd from QDomNode not implemented here");
    }

    friend bool operator==(const ConfigValueKbd& lhs, const ConfigValueKbd& rhs) {
        // Both the key sequence and the value of the base class must be consistent!
        // TODO(XXX): Fix this error prone design!!
        DEBUG_ASSERT((lhs.m_keys == rhs.m_keys) ==
                (static_cast<const ConfigValue&>(lhs) == static_cast<const ConfigValue&>(rhs)));
        return lhs.m_keys == rhs.m_keys;
    }

   private:
    QKeySequence m_keys;
};

inline bool operator!=(const ConfigValueKbd& lhs, const ConfigValueKbd& rhs) {
    return !(lhs == rhs);
}

template <class ValueType> class ConfigObject {
  public:
    ConfigObject(const QString& file);
    ConfigObject(const QDomNode& node);
    ~ConfigObject();

    // Sets the value v for key k, over-writing pre-existing values.
    void set(const ConfigKey& k, const ValueType& v);

    // Returns the ValueType entry for k. If k is not present, returns
    // ValueType().
    ValueType get(const ConfigKey& k) const;

    // Returns true if key is present.
    bool exists(const ConfigKey& key) const;

    // Removes key from ConfigObject. Returns whether key was present.
    bool remove(const ConfigKey& key);

    // Returns the string value associated with key. If key is not present,
    // returns QString().
    QString getValueString(const ConfigKey& key) const;

    // Sets the value for key to ValueType(value), over-writing pre-existing
    // values. ResultType is serialized to string on a per-type basis.
    template <class ResultType>
    void setValue(const ConfigKey& key, const ResultType& value);

    // Returns the value for key, converted to ResultType. If key is not present
    // or the value cannot be converted to ResultType, returns ResultType().
    template <class ResultType>
    ResultType getValue(const ConfigKey& key) const {
        return getValue<ResultType>(key, ResultType());
    }
    QString getValue(const ConfigKey& key) const {
        return getValueString(key);
    }

    // Returns the value for key, converted to ResultType. If key is not present
    // or the value cannot be converted to ResultType, returns default_value.
    template <class ResultType>
    ResultType getValue(const ConfigKey& key, const ResultType& default_value) const;
    QString getValue(const ConfigKey& key, const char* default_value) const;

    QMultiHash<ValueType, ConfigKey> transpose() const;

    void reopen(const QString& file);
    bool save();

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

    QSet<QString> getGroups();
    QList<ConfigKey> getKeysWithGroup(QString group) const;

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

#endif // PREFERENCES_CONFIGOBJECT_H
