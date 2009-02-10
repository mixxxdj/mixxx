#include <QtCore>
#include <QtXml>
#include "midimessage.h"

/** MidiMessage constructor */
MidiMessage::MidiMessage(MidiType miditype, int midino, int midichannel)
{
    m_midiType = miditype;
    m_midiNo = midino;
    m_midiChannel = midichannel;
}

/** Constructor that unserializes a MidiMessage object from a <control> node block in our
    MIDI mapping XML file.
*/
MidiMessage::MidiMessage(QDomElement& controlNode)
{
    // For each control

    QString midiType = controlNode.firstChildElement("miditype").text();
    QString midiNo = controlNode.firstChildElement("midino").text();
    QString midiChan = controlNode.firstChildElement("midichan").text();
    bool ok;

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

    //Use hex conversions
    m_midiNo = midiNo.toInt(&ok, 16);
    m_midiChannel = midiChan.toInt(&ok, 16);
}

void MidiMessage::serializeToXML(QDomElement& controlNode) const
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
    controlNode.appendChild(tagNode);

    //Midi no
    tagNode = nodeMaker.createElement("midino");
    text = nodeMaker.createTextNode(QString("0x%1").arg(this->getMidiNo(), 0, 16)); //Base 16
    tagNode.appendChild(text);
    controlNode.appendChild(tagNode);

    //Midi channel
    tagNode = nodeMaker.createElement("midichan");
    text = nodeMaker.createTextNode(QString("0x%1").arg(this->getMidiChannel(), 0, 16)); //Base 16
    tagNode.appendChild(text);
    controlNode.appendChild(tagNode);

}
