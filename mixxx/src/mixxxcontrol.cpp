#include <QtCore>
#include <QtXml>
#include "mixxxcontrol.h"

/** MixxxControl constructor */
MixxxControl::MixxxControl(QString controlobject_group, QString controlobject_value, 
                         QString controlobject_description, MidiOption midioption)
{
    m_strCOGroup = controlobject_group;
    m_strCOValue = controlobject_value;
    m_strCODescription = controlobject_description;
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
    QDomElement descriptionNode = parentNode.firstChildElement("description");

    m_strCOGroup = groupNode.text();
    m_strCOValue = keyNode.text();
    m_strCODescription = descriptionNode.text();

    QDomElement optionsNode = parentNode.firstChildElement("options");

    // At the moment, use one element, in future iterate through options
    QString strMidiOption;
    if (optionsNode.hasChildNodes()) {
        strMidiOption = optionsNode.firstChild().nodeName().toLower();
    } else {
        strMidiOption = "normal";
    }

    if (strMidiOption == "normal")  // can't switch() on a string
        m_midiOption = MIDI_OPT_NORMAL;
    else if (strMidiOption == "invert")
        m_midiOption = MIDI_OPT_INVERT;
    else if (strMidiOption == "rot64")
        m_midiOption = MIDI_OPT_ROT64;
    else if (strMidiOption == "rot64inv")
        m_midiOption = MIDI_OPT_ROT64_INV;
    else if (strMidiOption == "rot64fast")
        m_midiOption = MIDI_OPT_ROT64_FAST;
    else if (strMidiOption == "diff")
        m_midiOption = MIDI_OPT_DIFF;
    else if (strMidiOption == "button")
        m_midiOption = MIDI_OPT_BUTTON;
    else if (strMidiOption == "switch")
        m_midiOption = MIDI_OPT_SWITCH;
    else if (strMidiOption == "hercjog")
        m_midiOption = MIDI_OPT_HERC_JOG;
    else if (strMidiOption == "spread64")
        m_midiOption = MIDI_OPT_SPREAD64;
    else if (strMidiOption == "selectknob")
        m_midiOption = MIDI_OPT_SELECTKNOB;
    else if (strMidiOption == "soft-takeover")
        m_midiOption = MIDI_OPT_SOFT_TAKEOVER;
    else if (strMidiOption == "script-binding")
        m_midiOption = MIDI_OPT_SCRIPT;
    else {
        m_midiOption = MIDI_OPT_NORMAL;
        qWarning() << "Unknown midioption" << strMidiOption << "in" << __FILE__;
    }
    
    //Parse threshold stuff, only used for output.
    if (isOutputNode) {
        
        //TODO: Parse threshold stuff
        QDomElement minNode = parentNode.firstChildElement("minimum");
        QDomElement maxNode = parentNode.firstChildElement("maximum");
        
        bool ok = false;
        if (!minNode.isNull()) {
            m_thresholdMinimum = minNode.text().toFloat(&ok);
        } else {
            ok = false;
        }

        if (!ok) //If not a float, or node wasn't defined
            m_thresholdMinimum = 0.0f;
            
        if (!maxNode.isNull()) {
            m_thresholdMaximum = maxNode.text().toFloat(&ok);
        } else {
            ok = false;
        }

        if (!ok) //If not a float, or node wasn't defined
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

    //Control object description
    tagNode = nodeMaker.createElement("description");
    text = nodeMaker.createTextNode(this->getControlObjectDescription());
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);

    //Midi option (slightly different format)
    QDomElement optionsNode = nodeMaker.createElement("options");
    QString strMidiOption;
    int iMidiOption = this->getMidiOption();
    if (iMidiOption == MIDI_OPT_NORMAL) // can't switch() on a string
        strMidiOption = "normal";
    else if (iMidiOption == MIDI_OPT_INVERT)
        strMidiOption = "invert";
    else if (iMidiOption == MIDI_OPT_ROT64)
        strMidiOption = "rot64";
    else if (iMidiOption == MIDI_OPT_ROT64_INV)
        strMidiOption = "rot64inv";
    else if (iMidiOption == MIDI_OPT_ROT64_FAST)
        strMidiOption = "rot64fast";
    else if (iMidiOption == MIDI_OPT_DIFF)
        strMidiOption = "diff";
    else if (iMidiOption == MIDI_OPT_BUTTON)
        strMidiOption = "button";
    else if (iMidiOption == MIDI_OPT_SWITCH)
        strMidiOption = "switch";
    else if (iMidiOption == MIDI_OPT_HERC_JOG)
        strMidiOption = "hercjog";
    else if (iMidiOption == MIDI_OPT_SPREAD64)
        strMidiOption = "spread64";
    else if (iMidiOption == MIDI_OPT_SELECTKNOB)
        strMidiOption = "selectknob";
    else if (iMidiOption == MIDI_OPT_SOFT_TAKEOVER)
        strMidiOption = "soft-takeover";
    else if (iMidiOption == MIDI_OPT_SCRIPT)
        strMidiOption = "script-binding";
    else {
        strMidiOption = "Unknown";
        qWarning() << "Unknown midioption in" << __FILE__;
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


uint qHash(const MixxxControl& key)
{
    return (qHash(key.getControlObjectGroup() + key.getControlObjectValue()));
}
