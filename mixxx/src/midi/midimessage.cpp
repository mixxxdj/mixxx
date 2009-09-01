#include <QtCore>
#include <QtXml>
#include "midimessage.h"

/** MidiMessage constructor */
MidiMessage::MidiMessage(MidiStatusByte status, int midino, char midichannel)
{
    //Register this class with QT so we can use this bad boy in signals/slots.
    qRegisterMetaType<MidiMessage>("MidiMessage");

    m_midiStatusByte = status;
    m_midiNo = midino;
    m_midiStatusByte |= midichannel; //Jam midichannel into low nibble of status byte.

    m_midiByte2On = 0x7F; /** Defaults according to our MIDI XML spec on the wiki */
    m_midiByte2Off = 0x00;
}

/** Constructor that unserializes a MidiMessage object from a <control> or <output>
    node block in our MIDI mapping XML file.
*/
MidiMessage::MidiMessage(QDomElement& parentNode)
{
    // For each control

    QString midiStatus = parentNode.firstChildElement("status").text();
    QString midiNo = parentNode.firstChildElement("midino").text();
    QString midiOn = parentNode.firstChildElement("on").text();
    QString midiOff = parentNode.firstChildElement("off").text();

    bool ok = false;

    //Use QString with toInt base of 0 to auto convert hex values
    m_midiNo = midiNo.toInt(&ok, 0);
    if (!ok)
        m_midiNo = 0x00;

    m_midiStatusByte = midiStatus.toInt(&ok, 0);
    if (!ok)
        m_midiStatusByte = 0x00;

    m_midiByte2On = midiOn.toInt(&ok, 0);
    if (!ok)
        m_midiByte2On = 0x7F;

    m_midiByte2Off = midiOff.toInt(&ok, 0);
    if (!ok)
        m_midiByte2Off = 0x00;
}

void MidiMessage::serializeToXML(QDomElement& parentNode, bool isOutputNode) const
{
    QDomText text;
    QDomDocument nodeMaker;
    QDomElement tagNode;

    //Midi status byte
    tagNode = nodeMaker.createElement("status");
    QString strMidiStatus;
    text = nodeMaker.createTextNode(QString("0x%1").arg(this->getMidiStatusByte(), 0, 16));
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);

    //Midi no
    tagNode = nodeMaker.createElement("midino");
    text = nodeMaker.createTextNode(QString("0x%1").arg(this->getMidiNo(), 0, 16)); //Base 16
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);

    //If we're writing to an <output> node, include the <on> and <off> MIDI "Byte 2" info
    if (isOutputNode)
    {
        //Midi Byte 2 for to turn on LEDs
        tagNode = nodeMaker.createElement("on");
        text = nodeMaker.createTextNode(QString("0x%1").arg(this->getMidiByte2On(), 0, 16)); //Base 16
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);

        //Midi Byte 2 for to turn off LEDs
        tagNode = nodeMaker.createElement("off");
        text = nodeMaker.createTextNode(QString("0x%1").arg(this->getMidiByte2Off(), 0, 16)); //Base 16
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);
    }
}

QString MidiMessage::toString() const
{
    QString channel = "Channel: " + QString("%1").arg(this->getMidiChannel()) + "\n";
    QString status = "Status: " + QString("%1").arg(this->getMidiStatusByte()) + "\n";
    QString midino = "Number: " + QString("%1").arg(this->getMidiNo()) + "\n";

    QString hooah = channel + status + midino;
    return hooah;
}

uint qHash(const MidiMessage& key)
{
    //& with 0xF0 to ignore the channel bits for comparison purposes.
    if ((key.getMidiStatusByte() & 0xF0) == MIDI_STATUS_PITCH_BEND) { 
        //Ignore midino for pitch bend messages because those bits are actually part of the 14-bit pitch bend payload.
        return key.getMidiStatusByte(); 
    }
    else
        return (key.getMidiByte2On() << 16) | (key.getMidiStatusByte() << 8) | key.getMidiNo();
}
