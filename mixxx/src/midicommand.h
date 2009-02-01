#ifndef _MIDICONTROLMAPPING_H_
#define _MIDICONTROLMAPPING_H_

#include <QDebug>
#include "configobject.h"

/** Note: The hash table in the MIDI mapping class maps MidiCommands onto MidiControls! */

/** The key used in the MIDI mapping hash table */
class MidiCommand
{
    public:
        MidiCommand(MidiType miditype=MIDI_EMPTY, int midino=0, int midichannel=0);
        MidiCommand(QDomElement& controlNode);
        ~MidiCommand() {};
        void setMidiType(MidiType type) { m_midiType = type; };
        void setMidiNo(int midino) { m_midiNo = midino; };
        void setMidiChannel(int midichannel) { m_midiChannel = midichannel; };
        MidiType getMidiType() const { return m_midiType; } ;
        int getMidiNo() const { return m_midiNo; };
        int getMidiChannel() const { return m_midiChannel; };
        void serializeToXML(QDomElement& controlNode) const;        
        bool operator==(MidiCommand& other) {
            return ((m_midiType == other.getMidiType()) &&
                    (m_midiNo == other.getMidiNo()) &&
                    (m_midiChannel == other.getMidiChannel()));
        };
        
    private:
        MidiType m_midiType;
        int m_midiNo;
        int m_midiChannel;        
};

inline bool operator<(const MidiCommand &first, const MidiCommand &second)
{
     int firstval = (first.getMidiChannel() * 128) + (int)first.getMidiType() * (128*16) + first.getMidiNo();
     int secondval = (second.getMidiChannel() * 128) + (int)second.getMidiType() * (128*16) + second.getMidiNo();
     return firstval < secondval;
}

/* Linker error wtf?
QDebug operator<<(QDebug dbg, MidiCommand& command)
{
    dbg.space() << command.getMidiType();
    dbg.space() << command.getMidiNo();
    dbg.space() << command.getMidiChannel();

    return dbg.space();
}*/

/** Value in the hash table sense */
class MidiControl
{
    public:
        MidiControl(QString controlobject_group="", QString controlobject_value="",
                         MidiOption midioption=MIDI_OPT_NORMAL);
        MidiControl(QDomElement& controlNode);
        ~MidiControl() {};
        void setControlObjectGroup(QString controlobject_group) { m_strCOGroup = controlobject_group; };
        void setControlObjectValue(QString controlobject_value) { m_strCOValue = controlobject_value; };
        void setMidiOption(MidiOption midioption) { m_midiOption = midioption; };
        QString getControlObjectGroup() const { return m_strCOGroup; };
        QString getControlObjectValue() const { return m_strCOValue; };
        MidiOption getMidiOption() const { return m_midiOption; };
        void serializeToXML(QDomElement& controlNode) const;
        bool operator==(MidiControl& other) {
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
QDebug operator<<(QDebug dbg, MidiControl& control)
{
    dbg.space() << control.getControlObjectGroup();
    dbg.space() << control.getControlObjectValue();
    dbg.space() << control.getMidiOption();

    return dbg.space();
}*/

#endif

