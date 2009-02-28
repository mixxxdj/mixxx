#include <QtCore>
#include <QtXml>
#include "midimessage.h"

/** MidiMessage constructor */
MidiMessage::MidiMessage(MidiType miditype, int midino, int midichannel)
{
    m_midiType = miditype;
    m_midiNo = midino;
    m_midiChannel = midichannel;
    
    m_midiByte2On = 0x7F; /** Defaults according to our MIDI XML spec on the wiki */
    m_midiByte2Off = 0x00;
}

/** Constructor that unserializes a MidiMessage object from a <control> or <output> 
    node block in our MIDI mapping XML file.
*/
MidiMessage::MidiMessage(QDomElement& parentNode)
{
    // For each control

    QString midiType = parentNode.firstChildElement("miditype").text();
    QString midiNo = parentNode.firstChildElement("midino").text();
    QString midiChan = parentNode.firstChildElement("midichan").text();
    QString midiOn = parentNode.firstChildElement("on").text();
    QString midiOff = parentNode.firstChildElement("off").text();

    bool ok = false;

    if (midiType == "Key")
        m_midiType = MIDI_KEY;
    else if (midiType == "Ctrl")
        m_midiType = MIDI_CTRL;
    else if (midiType == "Pitch")
        m_midiType = MIDI_PITCH;
    else {
        qDebug() << "Warning: Unknown miditype" << midiType << "in" << __FILE__;
        m_midiType = MIDI_KEY;
    }

    //Use QString with toInt base of 0 to auto convert hex values
    m_midiNo = midiNo.toInt(&ok, 0);
    if (!ok)
        m_midiNo = 0x00;
        
    m_midiChannel = midiChan.toInt(&ok, 0);
    if (!ok)
        m_midiChannel = 1;
        
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

    //Midi type
    tagNode = nodeMaker.createElement("miditype");
    QString strMidiType;
    int iMidiType = this->getMidiType();
    if (iMidiType == MIDI_KEY)
        strMidiType = "Key";
    else if (iMidiType == MIDI_CTRL)
        strMidiType = "Ctrl";
    else if (iMidiType == MIDI_PITCH)
        strMidiType = "Pitch";
    else {
        strMidiType = "Unknown";
        qDebug() << "Warning: Unknown miditype in" << __FILE__;
    }
    text = nodeMaker.createTextNode(strMidiType);
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);

    //Midi no
    tagNode = nodeMaker.createElement("midino");
    text = nodeMaker.createTextNode(QString("0x%1").arg(this->getMidiNo(), 0, 16)); //Base 16
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);

    //Midi channel
    tagNode = nodeMaker.createElement("midichan");
    text = nodeMaker.createTextNode(QString("0x%1").arg(this->getMidiChannel(), 0, 16)); //Base 16
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
