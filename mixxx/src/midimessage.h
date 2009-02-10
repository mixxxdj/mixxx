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
        MidiMessage(QDomElement& controlNode);
        ~MidiMessage() {};
        void setMidiType(MidiType type) { m_midiType = type; };
        void setMidiNo(int midino) { m_midiNo = midino; };
        void setMidiChannel(int midichannel) { m_midiChannel = midichannel; };
        MidiType getMidiType() const { return m_midiType; } ;
        int getMidiNo() const { return m_midiNo; };
        int getMidiChannel() const { return m_midiChannel; };
        void serializeToXML(QDomElement& controlNode) const;        
        bool operator==(MidiMessage& other) {
            return ((m_midiType == other.getMidiType()) &&
                    (m_midiNo == other.getMidiNo()) &&
                    (m_midiChannel == other.getMidiChannel()));
        };
        
    private:
        MidiType m_midiType;
        int m_midiNo;
        int m_midiChannel;        
};

inline bool operator<(const MidiMessage &first, const MidiMessage &second)
{
     int firstval = (first.getMidiChannel() * 128) + (int)first.getMidiType() * (128*16) + first.getMidiNo();
     int secondval = (second.getMidiChannel() * 128) + (int)second.getMidiType() * (128*16) + second.getMidiNo();
     return firstval < secondval;
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

