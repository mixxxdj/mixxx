#include <QtCore>
#include <QtXml>
#include "midicommand.h"

/** MidiCommand constructor */
MidiCommand::MidiCommand(MidiType miditype, int midino, int midichannel)
{
    m_midiType = miditype;
    m_midiNo = midino;
    m_midiChannel = midichannel;
}

/** Constructor that unserializes a MidiCommand object from a <control> node block in our 
    MIDI mapping XML file.
*/
MidiCommand::MidiCommand(QDomElement& controlNode)
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

void MidiCommand::serializeToXML(QDomElement& controlNode) const
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

/** MidiControl constructor */
MidiControl::MidiControl(QString controlobject_group, QString controlobject_value,
                         MidiOption midioption)
{
    m_strCOGroup = controlobject_group;
    m_strCOValue = controlobject_value;
    m_midiOption = midioption;
}

/** Constructor that unserializes a MidiControl object from a <control> node block in our 
    MIDI mapping XML file.
*/
MidiControl::MidiControl(QDomElement& controlNode)
{
    QDomElement groupNode = controlNode.firstChildElement("group");
    QDomElement keyNode = controlNode.firstChildElement("key");
    
    m_strCOGroup = groupNode.text();
    m_strCOValue = keyNode.text();

    QDomElement optionsNode = controlNode.firstChildElement("options");
    // At the moment, use one element, in future iterate through options
    QString strMidiOption;
    if (optionsNode.hasChildNodes()) {
        strMidiOption = optionsNode.firstChild().nodeName();
    } else {
        strMidiOption = "Normal";
    }
    
    if (strMidiOption == "Normal")
        m_midiOption = MIDI_OPT_NORMAL;
    else if (strMidiOption == "Invert")
        m_midiOption = MIDI_OPT_INVERT;
    else if (strMidiOption == "Rot64")
        m_midiOption = MIDI_OPT_ROT64;
    else if (strMidiOption == "Rot64inv")
        m_midiOption = MIDI_OPT_ROT64_INV;
    else if (strMidiOption == "Rot64fast")
        m_midiOption = MIDI_OPT_ROT64_FAST;
    else if (strMidiOption == "Diff")
        m_midiOption = MIDI_OPT_DIFF;
    else if (strMidiOption == "Button")
        m_midiOption = MIDI_OPT_BUTTON;
    else if (strMidiOption == "Switch")
        m_midiOption = MIDI_OPT_SWITCH;
    else if (strMidiOption == "Hercjog")
        m_midiOption = MIDI_OPT_HERC_JOG;
    else if (strMidiOption == "Spread64")
        m_midiOption = MIDI_OPT_SPREAD64;
    else if (strMidiOption == "SelectKnob")
        m_midiOption = MIDI_OPT_SELECTKNOB;
    else if (strMidiOption == "Script-Binding")
        m_midiOption = MIDI_OPT_SCRIPT;
    else {
        m_midiOption = MIDI_OPT_NORMAL;
        qDebug() << "Warning: Unknown midioption" << strMidiOption << "in" << __FILE__;
    }
}

void MidiControl::serializeToXML(QDomElement& controlNode) const
{
    QDomText text;
    QDomDocument nodeMaker;
    QDomElement tagNode;
         
    //Control object group
    tagNode = nodeMaker.createElement("group");
    text = nodeMaker.createTextNode(this->getControlObjectGroup());
    tagNode.appendChild(text);
    controlNode.appendChild(tagNode);
    
    //Control object value
    tagNode = nodeMaker.createElement("key"); //WTF worst name ever
    text = nodeMaker.createTextNode(this->getControlObjectValue()); 
    tagNode.appendChild(text);
    controlNode.appendChild(tagNode);

    //Midi option (slightly different format)
    QDomElement optionsNode = nodeMaker.createElement("options");
    QString strMidiOption;
    int iMidiOption = this->getMidiOption();
    if (iMidiOption == MIDI_OPT_NORMAL)
        strMidiOption = "Normal";
    else if (iMidiOption == MIDI_OPT_INVERT)
        strMidiOption = "Invert";
    else if (iMidiOption == MIDI_OPT_ROT64)
        strMidiOption = "Rot64";
    else if (iMidiOption == MIDI_OPT_ROT64_INV)
        strMidiOption = "Rot64inv";
    else if (iMidiOption == MIDI_OPT_ROT64_FAST)
        strMidiOption = "Rot64fast";
    else if (iMidiOption == MIDI_OPT_DIFF)
        strMidiOption = "Diff";
    else if (iMidiOption == MIDI_OPT_BUTTON)
        strMidiOption = "Button";
    else if (iMidiOption == MIDI_OPT_SWITCH)
        strMidiOption = "Switch";
    else if (iMidiOption == MIDI_OPT_HERC_JOG)
        strMidiOption = "Hercjog";
    else if (iMidiOption == MIDI_OPT_SPREAD64)
        strMidiOption = "Spread64";
    else if (iMidiOption == MIDI_OPT_SELECTKNOB)
        strMidiOption = "SelectKnob";
    else if (iMidiOption == MIDI_OPT_SCRIPT)
        strMidiOption = "Script-Binding";
    else {
        strMidiOption = "Unknown";
        qDebug() << "Warning: Unknown midioption in" << __FILE__;
    }
    
    QDomElement singleOption = nodeMaker.createElement(strMidiOption);
    optionsNode.appendChild(singleOption);
    controlNode.appendChild(tagNode);    
}