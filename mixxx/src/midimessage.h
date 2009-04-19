#ifndef _MIDIMESSAGE_H_
#define _MIDIMESSAGE_H_

#include <QDebug>
#include "configobject.h"

/** Note: The hash table in the MIDI mapping class maps MidiMessages onto MidiControls! */

/** First byte of a MIDI message (aka Status Byte). The nibbles that are "0" in this
    enum are actually the channel nibble.
*/
typedef enum {
    MIDI_STATUS_NOTE_OFF   = 0x80,
    MIDI_STATUS_NOTE_ON    = 0x90,    
    MIDI_STATUS_CC         = 0xB0,
    MIDI_STATUS_PITCH_BEND = 0xE0,
} MidiStatusByte;
  
  /** The key used in the MIDI mapping hash table */
  class MidiMessage
  {
      public:
          MidiMessage(MidiStatusByte status=MIDI_STATUS_NOTE_ON, int midino=0, char midichannel=0);
          MidiMessage(QDomElement& parentNode);
          ~MidiMessage() {};
          void setMidiStatusByte(MidiStatusByte status) { m_midiStatusByte = status; };
          void setMidiNo(unsigned short midino) { m_midiNo = midino; };
          void setMidiChannel(unsigned short midichannel) { m_midiStatusByte |= midichannel; };
          unsigned short getMidiStatusByte() const { return m_midiStatusByte; } ;
          unsigned short getMidiNo() const { return m_midiNo; };
          unsigned short getMidiChannel() const { return m_midiStatusByte & 0x0F; };
          unsigned short getMidiByte2On() const { return m_midiByte2On; };
          unsigned short getMidiByte2Off() const { return m_midiByte2Off; };
          void serializeToXML(QDomElement& parentNode, bool isOutputNode=false) const;
          QString toString() const;
          bool operator==(const MidiMessage& other) const {
            //Compare high bits, which ignores the channel
            if ((this->getMidiStatusByte() & 0xF0) == MIDI_STATUS_PITCH_BEND) {
                //Ignore midiNo for pitch messages because that byte is part of the message payload.
                //(See the MIDI spec.)
                return (this->getMidiStatusByte() == other.getMidiStatusByte());
            }
            else {
                return ((m_midiStatusByte == other.getMidiStatusByte()) &&
                    (m_midiNo == other.getMidiNo()));
            }
          };
  
      private:
         unsigned short m_midiStatusByte; /** Complete 8-bit status byte (including category and channel). */
         unsigned short m_midiNo;         /** The second MIDI byte. */
  
          /** These next parameters are used when sending this MidiMessage as output.
              m_midiOn or m_midiOff is sent as "Byte 2" (third byte) in a MidiMessage depending
              on whether we want to illuminate an LED or turn it off or whatever. */
        unsigned short m_midiByte2On;  /** Sent as "Byte 2" in the MIDI message when we need to turn on an LED. */
        unsigned short m_midiByte2Off; /** Sent as "Byte 2" in the MIDI message when we need to turn off an LED. */
  
  };
  
/** Hash function so we can use MidiMessage in a QHash table */
uint qHash(const MidiMessage& key);

inline QDataStream& operator<<(QDataStream & stream, const MidiMessage &first)
{
     return stream << first.toString();
}

/* Linker error wtf?
QDebug operator<<(QDebug dbg, MidiMessage& command)
{
    dbg.space() << command.getMidiStatusByte();
    dbg.space() << command.getMidiNo();
    dbg.space() << command.getMidiChannel();

    return dbg.space();
}*/

#endif

