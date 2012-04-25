#ifndef MIXXXCONTROL_H
#define MIXXXCONTROL_H

#include <QDebug>
#include <QVariant>
#include "controlobject.h"

class MixxxControl {
    public:
        MixxxControl(QString controlobject_group="", QString controlobject_item="",
                     QString controlobject_description="");
//         MixxxControl(QDomElement& controlNode, bool isOutputNode=false);
        ~MixxxControl() {};
        void setGroup(QString controlobject_group) { m_sGroup = controlobject_group; };
        void setItem(QString controlobject_item) { m_sItem = controlobject_item; };
        void setDescription(QString controlobject_description) { m_sDescription = controlobject_description; };

        QString group() const { return m_sGroup; };
        QString item() const { return m_sItem; };
        QString description() const { return m_sDescription; };
        ControlObject* getControlObject() const {
            return ControlObject::getControl(ConfigKey(m_sGroup, m_sItem));
        };
//         void serializeToXML(QDomElement& parentNode, bool isOutputNode=false) const;
        bool operator==(const MixxxControl& other) const {
            return ((m_sGroup == other.group()) &&
                    (m_sItem == other.item()));
        };
        bool isNull() { return (m_sGroup == "" && m_sItem == ""); };
    private:
        QString m_sGroup;
        QString m_sItem;
        QString m_sDescription;
};

inline bool operator<(const MixxxControl &first, const MixxxControl &second)
{
   return ((first.group() + first.item()) <
            (second.group() + second.item()));
}

inline bool groupLessThan(const MixxxControl &first, const MixxxControl &second)
{
    return (first.group() < second.group());
}

/** Hash function needed so we can use MixxxControl in a QHash table */
uint qHash(const MixxxControl& key);

/*  The below breaks linking.
QDebug operator<<(QDebug dbg, MixxxControl& control)
{
    dbg.space() << control.group();
    dbg.space() << control.item();

    return dbg.space();
}*/

#endif
