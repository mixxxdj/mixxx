#ifndef _MIXXXCONTROL_H_
#define _MIXXXCONTROL_H_

#include <QDebug>
#include "configobject.h"

typedef enum {
    MIDI_OPT_NORMAL           = 0,
    MIDI_OPT_INVERT           = 1,
    MIDI_OPT_ROT64            = 2,
    MIDI_OPT_ROT64_INV        = 3,
    MIDI_OPT_ROT64_FAST       = 4,
    MIDI_OPT_DIFF             = 5,
    MIDI_OPT_BUTTON           = 6, // Button Down (!=00) and Button Up (00) events happen together
    MIDI_OPT_SWITCH           = 7, // Button Down (!=00) and Button Up (00) events happen seperately
    MIDI_OPT_HERC_JOG         = 8, // Generic hercules wierd range correction
    MIDI_OPT_SPREAD64         = 9, // Accelerated difference from 64
    MIDI_OPT_SELECTKNOB       = 10,// Relative knob which can be turned forever and outputs a signed value.

    MIDI_OPT_SOFT_TAKEOVER    = 40,// Prevents sudden changes when hardware position differs from software value
    
    MIDI_OPT_SCRIPT           = 50,// Maps a MIDI control to a custom MixxxScript function
} MidiOption;

/** Note: The hash table in the MIDI mapping class maps MidiCommands onto MixxxControls! */

class MixxxControl
{
    public:
        MixxxControl(QString controlobject_group="", QString controlobject_value="",
                         QString controlobject_description="",
                         MidiOption midioption=MIDI_OPT_NORMAL);
        MixxxControl(QDomElement& controlNode, bool isOutputNode=false);
        ~MixxxControl() {};
        void setControlObjectGroup(QString controlobject_group) { m_strCOGroup = controlobject_group; };
        void setControlObjectValue(QString controlobject_value) { m_strCOValue = controlobject_value; };
        void setControlObjectDescription(QString controlobject_description) { m_strCODescription = controlobject_description; };
        void setMidiOption(MidiOption midioption) { m_midiOption = midioption; };
        void setThresholdMinimum(float min) { m_thresholdMinimum = min; };
        void setThresholdMaximum(float max) { m_thresholdMaximum = max; };
        QString getControlObjectGroup() const { return m_strCOGroup; };
        QString getControlObjectValue() const { return m_strCOValue; };
        QString getControlObjectDescription() const { return m_strCODescription; };
        MidiOption getMidiOption() const { return m_midiOption; };
        float getThresholdMinimum() const { return m_thresholdMinimum; };
        float getThresholdMaximum() const { return m_thresholdMaximum; };
        void serializeToXML(QDomElement& parentNode, bool isOutputNode=false) const;
        bool operator==(const MixxxControl& other) const {
            return ((m_strCOGroup == other.getControlObjectGroup()) &&
                    (m_strCOValue == other.getControlObjectValue()) &&
                    (m_strCODescription == other.getControlObjectDescription()) &&
                    (m_midiOption == other.getMidiOption()));
        };
        bool isNull() { return (m_strCOGroup == "" && m_strCOValue == ""); };
    private:
        QString m_strCOGroup;
        QString m_strCOValue;
        QString m_strCODescription;
        MidiOption m_midiOption;
        
        /** These next parameters are used for MIDI output, when we're monitoring
            the value of some control and sending output when it hits some threshold
            (and stuff like that). */
        float m_thresholdMinimum;
        float m_thresholdMaximum; 
};

inline bool operator<(const MixxxControl &first, const MixxxControl &second)
{
   return ((first.getControlObjectGroup() + first.getControlObjectValue()) < 
            (second.getControlObjectGroup() + second.getControlObjectValue()));
}
  
/** Hash function needed so we can use MixxxControl in a QHash table */
uint qHash(const MixxxControl& key);

/*
QDebug operator<<(QDebug dbg, MixxxControl& control)
{
    dbg.space() << control.getControlObjectGroup();
    dbg.space() << control.getControlObjectValue();
    dbg.space() << control.getMidiOption();

    return dbg.space();
}*/

#endif

