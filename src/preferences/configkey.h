#pragma once

#include <QHash>
#include <QMetaType>
#include <QString>
#include <QtDebug>

#include "util/compatibility/qhash.h"

// Class for the key for a specific configuration element. A key consists of a
// group and an item.
//
// NOTE: new ConfigKey's item should use the `snake_case` formatting
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
inline qhash_seed_t qHash(
        const ConfigKey& key,
        qhash_seed_t seed = 0) {
    return qHash(key.group, seed) ^
            qHash(key.item, seed);
}
