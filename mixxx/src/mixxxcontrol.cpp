#include <QtCore>
#include <QtXml>
#include "mixxxcontrol.h"

/** MixxxControl constructor */
MixxxControl::MixxxControl(QString controlobject_group, QString controlobject_value,
                         MidiOption midioption)
{
    m_strCOGroup = controlobject_group;
    m_strCOValue = controlobject_value;
    m_midiOption = midioption;
    
    m_thresholdMinimum = 0.0f;
    m_thresholdMaximum = 1.0f;
}

/** Constructor that unserializes a MixxxControl object from a <control> or <output>
    node block in our MIDI mapping XML file.
*/
MixxxControl::MixxxControl(QDomElement& parentNode, bool isOutputNode)
{
    QDomElement groupNode = parentNode.firstChildElement("group");
    QDomElement keyNode = parentNode.firstChildElement("key");

    m_strCOGroup = groupNode.text();
    m_strCOValue = keyNode.text();

    QDomElement optionsNode = parentNode.firstChildElement("options");

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
    else if (strMidiOption == "HercJog")
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
    
    //Parse threshold stuff, only used for output.
    if (isOutputNode) {
        
        //TODO: Parse threshold stuff
        QDomElement minNode = parentNode.firstChildElement("minimum");
        QDomElement maxNode = parentNode.firstChildElement("maximum");
        
        bool ok = true;
        m_thresholdMinimum = minNode.text().toFloat(&ok);
        if (!ok)
            m_thresholdMinimum = 0.0f;
        m_thresholdMaximum = maxNode.text().toFloat(&ok);
        if (!ok)
            m_thresholdMaximum = 1.0f;
    }
}

void MixxxControl::serializeToXML(QDomElement& parentNode, bool isOutputNode) const
{
    QDomText text;
    QDomDocument nodeMaker;
    QDomElement tagNode;

    //Control object group
    tagNode = nodeMaker.createElement("group");
    text = nodeMaker.createTextNode(this->getControlObjectGroup());
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);

    //Control object value
    tagNode = nodeMaker.createElement("key"); //WTF worst name ever
    text = nodeMaker.createTextNode(this->getControlObjectValue());
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);

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
        strMidiOption = "HercJog";
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
    parentNode.appendChild(optionsNode);

    //If we're writing to an <output> block, write our threshold params
    if (isOutputNode)
    {
        //TODO: Write threshold blocks
        tagNode = nodeMaker.createElement("minimum");
        text = nodeMaker.createTextNode(QString("%1").arg(this->getThresholdMinimum()));
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);

        tagNode = nodeMaker.createElement("maximum");
        text = nodeMaker.createTextNode(QString("%1").arg(this->getThresholdMaximum()));
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);
    }
}
