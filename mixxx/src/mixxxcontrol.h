/**
* @file mixxxcontrol.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Wed 11 Apr 2012
* @brief Mixxx shared global control
*
* (For Control 2.0:)
* This class represents a shared global control for some aspect of Mixxx.
* Mixxx's various subsystems and threads communicate exclusively through
*   instances of this. (It is thread-safe.)
*
* At present, this class is just a sort of alias for ControlObjects and
*   is _not_ thread-safe.
*
*/

#ifndef MIXXXCONTROL_H
#define MIXXXCONTROL_H

#include <QDebug>
#include <QVariant>
#include "controlobject.h"

class MixxxControl
{
    public:
        MixxxControl(QString controlobject_group="", QString controlobject_item="",
                     QString controlobject_description="", bool internal = false,
                     bool readable = true, bool writeable = true);
//         MixxxControl(QDomElement& controlNode, bool isOutputNode=false);
        ~MixxxControl() {};
        void setGroup(QString controlobject_group) { m_sGroup = controlobject_group; };
        void setItem(QString controlobject_item) { m_sItem = controlobject_item; };
        void setDescription(QString controlobject_description) { m_sDescription = controlobject_description; };
        void setInternal(bool internal) { m_bInternal = internal; };
        void setRead(bool read) { m_bReadable = read; };
        void setWrite(bool write) { m_bWriteable = write; };
        void makeReadOnly() { m_bWriteable = false; };

        /*
        // Control 2.0 possibility
        void set(QVariant value) { m_value = value; };
        void setMax(QVariant value) { m_max = value; };
        void setMin(QVariant value) { m_min = value; };
        void setCenter(QVariant value) { m_center = value; };
        void setDefault(QVariant value) { m_default = value; };
        void reset() { m_value = m_default; };
        QVariant get() { return m_value; };
        QVariant max() { return m_max; };
        QVariant min() { return m_min; };
        QVariant center() { return m_center; };
        QVariant defaultValue() { return m_default; };
        */
        
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
        bool m_bReadable;
        bool m_bWriteable;
        bool m_bInternal;

        /*
        // Control 2.0 possibility
        QVariant m_value;
        QVariant m_max; // NULL for non-numeric values
        QVariant m_min; // NULL for non-numeric values
        QVariant m_center;  // NULL for non-numeric values
        QVariant m_default; // NULL if none
        */
};

inline bool operator<(const MixxxControl &first, const MixxxControl &second)
{
   return ((first.group() + first.item()) <
            (second.group() + second.item()));
}
  
/** Hash function needed so we can use MixxxControl in a QHash table */
uint qHash(const MixxxControl& key);

/*
QDebug operator<<(QDebug dbg, MixxxControl& control)
{
    dbg.space() << control.group();
    dbg.space() << control.item();

    return dbg.space();
}*/

#endif
