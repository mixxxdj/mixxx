#ifndef _MIDIMESSAGE_H_
#define _MIDIMESSAGE_H_

#include <QDebug>
#include "configobject.h"

/** Note: The hash table in the MIDI mapping class maps MidiMessages onto MidiControls! */

/** The key used in the MIDI mapping hash table */
class MidiMessage
{
    public:
        MidiMessage(MidiType miditype=MIDI_EMPTY, int midino=0, int midichannel=0);
        MidiMessage(QDomElement& parentNode);
        ~MidiMessage() {};
        void setMidiType(MidiType type) { m_midiType = type; };
        void setMidiNo(int midino) { m_midiNo = midino; };
        void setMidiChannel(int midichannel) { m_midiChannel = midichannel; };
        MidiType getMidiType() const { return m_midiType; } ;
        int getMidiNo() const { return m_midiNo; };
        int getMidiChannel() const { return m_midiChannel; };
        int getMidiByte2On() const { return m_midiByte2On; };
        int getMidiByte2Off() const { return m_midiByte2Off; };
        void serializeToXML(QDomElement& parentNode, bool isOutputNode=false) const;
        QString toString() const;
        bool operator==(MidiMessage& other) {
            return ((m_midiType == other.getMidiType()) &&
                    (m_midiNo == other.getMidiNo()) &&
                    (m_midiChannel == other.getMidiChannel()));
        };

    private:
        MidiType m_midiType;
        int m_midiNo;
        int m_midiChannel;

        /** These next parameters are used when sending this MidiMessage as output.
            m_midiOn or m_midiOff is sent as "Byte 2" (third byte) in a MidiMessage depending
            on whether we want to illuminate an LED or turn it off or whatever. */
        int m_midiByte2On;  /** Sent as "Byte 2" in the MIDI message when we need to turn on an LED. */
        int m_midiByte2Off; /** Sent as "Byte 2" in the MIDI message when we need to turn off an LED. */

};

inline bool operator<(const MidiMessage &first, const MidiMessage &second)
{
     int firstval = (first.getMidiChannel() * 128) + (int)first.getMidiType() * (128*16) + first.getMidiNo();
     int secondval = (second.getMidiChannel() * 128) + (int)second.getMidiType() * (128*16) + second.getMidiNo();
     return firstval < secondval;
}

inline QDataStream& operator<<(QDataStream & stream, const MidiMessage &first)
{
     return stream << first.toString();
}

/* Linker error wtf?
QDebug operator<<(QDebug dbg, MidiMessage& command)
{
    dbg.space() << command.getMidiType();
    dbg.space() << command.getMidiNo();
    dbg.space() << command.getMidiChannel();

    return dbg.space();
}*/

#endif

