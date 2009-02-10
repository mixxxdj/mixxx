#ifndef _MIXXXCONTROL_H_
#define _MIXXXCONTROL_H_

#include <QDebug>
#include "configobject.h"

/** Note: The hash table in the MIDI mapping class maps MidiCommands onto MixxxControls! */

/** Value in the hash table sense */
class MixxxControl
{
    public:
        MixxxControl(QString controlobject_group="", QString controlobject_value="",
                         MidiOption midioption=MIDI_OPT_NORMAL);
        MixxxControl(QDomElement& controlNode);
        ~MixxxControl() {};
        void setControlObjectGroup(QString controlobject_group) { m_strCOGroup = controlobject_group; };
        void setControlObjectValue(QString controlobject_value) { m_strCOValue = controlobject_value; };
        void setMidiOption(MidiOption midioption) { m_midiOption = midioption; };
        QString getControlObjectGroup() const { return m_strCOGroup; };
        QString getControlObjectValue() const { return m_strCOValue; };
        MidiOption getMidiOption() const { return m_midiOption; };
        void serializeToXML(QDomElement& controlNode) const;
        bool operator==(MixxxControl& other) {
            return ((m_strCOGroup == other.getControlObjectGroup()) &&
                    (m_strCOValue == other.getControlObjectValue()) &&
                    (m_midiOption == other.getMidiOption()));
        };     
    private:
        QString m_strCOGroup;
        QString m_strCOValue;
        MidiOption m_midiOption;
};

/*
QDebug operator<<(QDebug dbg, MixxxControl& control)
{
    dbg.space() << control.getControlObjectGroup();
    dbg.space() << control.getControlObjectValue();
    dbg.space() << control.getMidiOption();

    return dbg.space();
}*/

#endif

