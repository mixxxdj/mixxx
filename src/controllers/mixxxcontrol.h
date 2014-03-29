#ifndef MIXXXCONTROL_H
#define MIXXXCONTROL_H

#include <QtDebug>
#include <QVariant>
#include <QString>
#include <QDomElement>
#include <QHash>

#include "controlobject.h"

class MixxxControl {
  public:
    MixxxControl(QString group="", QString item="",
                 QString description="")
            : m_key(group, item),
              m_description(description) {
    }

    virtual ~MixxxControl() {}

    void setGroup(QString group) {
        m_key.group = group;
    }
    void setItem(QString item) {
        m_key.item = item;
    }

    void setDescription(QString description) {
        m_description = description;
    }

    inline const QString& group() const {
        return m_key.group;
    }

    inline const QString& item() const {
        return m_key.item;
    }

    inline const QString& description() const {
        return m_description;
    }

    inline const ConfigKey& key() const {
        return m_key;
    }
    ControlObject* getControlObject() const {
        return ControlObject::getControl(m_key);
    }

    bool operator==(const MixxxControl& other) const {
        return ((group() == other.group()) &&
                (item() == other.item()));
    }

    bool isNull() const {
        return group() == "" && item() == "";
    }

  private:
    ConfigKey m_key;
    QString m_description;
};

inline bool operator<(const MixxxControl& first, const MixxxControl& second) {
   return (first.group() + first.item()) < (second.group() + second.item());
}

// Hash function needed so we can use MixxxControl in a QHash table.
inline uint qHash(const MixxxControl& key) {
    return qHash(key.group()) + qHash(key.item());
}

#endif
