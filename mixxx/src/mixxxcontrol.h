#ifndef _MIXXXCONTROL_H_
#define _MIXXXCONTROL_H_

#include <QDebug>
#include "configobject.h"

/** Note: The hash table in the MIDI mapping class maps MidiCommands onto MixxxControls! */

class MixxxControl
{
    public:
        MixxxControl(QString controlobject_group="", QString controlobject_value="",
                         MidiOption midioption=MIDI_OPT_NORMAL);
        MixxxControl(QDomElement& controlNode, bool isOutputNode=false);
        ~MixxxControl() {};
        void setControlObjectGroup(QString controlobject_group) { m_strCOGroup = controlobject_group; };
        void setControlObjectValue(QString controlobject_value) { m_strCOValue = controlobject_value; };
        void setMidiOption(MidiOption midioption) { m_midiOption = midioption; };
        QString getControlObjectGroup() const { return m_strCOGroup; };
        QString getControlObjectValue() const { return m_strCOValue; };
        MidiOption getMidiOption() const { return m_midiOption; };
        float getThresholdMinimum() const { return m_thresholdMinimum; };
        float getThresholdMaximum() const { return m_thresholdMaximum; };
        void serializeToXML(QDomElement& parentNode, bool isOutputNode=false) const;
        bool operator==(MixxxControl& other) {
            return ((m_strCOGroup == other.getControlObjectGroup()) &&
                    (m_strCOValue == other.getControlObjectValue()) &&
                    (m_midiOption == other.getMidiOption()));
        };     
    private:
        QString m_strCOGroup;
        QString m_strCOValue;
        MidiOption m_midiOption;
        
        /** These next parameters are used for MIDI output, when we're monitoring
            the value of some control and sending output when it hits some threshold
            (and stuff like that). */
        float m_thresholdMinimum;
        float m_thresholdMaximum; 
};

inline bool operator<(const MixxxControl &first, const MixxxControl &second)
{
     //int firstval = (first.getMidiChannel() * 128) + (int)first.getMidiType() * (128*16) + first.getMidiNo();
     //int secondval = (second.getMidiChannel() * 128) + (int)second.getMidiType() * (128*16) + second.getMidiNo();
     //return firstval < secondval;
     return ((first.getControlObjectGroup() + first.getControlObjectValue()) < 
              (second.getControlObjectGroup() + second.getControlObjectValue()));
}

/*
QDebug operator<<(QDebug dbg, MixxxControl& control)
{
    dbg.space() << control.getControlObjectGroup();
    dbg.space() << control.getControlObjectValue();
    dbg.space() << control.getMidiOption();

    return dbg.space();
}*/

#endif

