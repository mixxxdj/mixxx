/**
* @file midicontroller.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Tue 7 Feb 2012
* @brief MIDI Controller base class
*
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "midicontroller.h"
#include "xmlparse.h"
#include "controlobject.h"
#include "errordialoghandler.h"

#include "../defs_controllers.h"   // For MIDI_MAPPING_EXTENSION

#define DEFAULT_OUTPUT_MAX  1.0f
#define DEFAULT_OUTPUT_MIN  0.0f    // Anything above 0 is "on"
#define DEFAULT_OUTPUT_ON   0x7F
#define DEFAULT_OUTPUT_OFF  0x00

MidiController::MidiController() : Controller() {
    m_bMidiLearn = false;
}

MidiController::~MidiController(){
    destroyOutputHandlers();
//     close(); // I wish I could put this here to enforce it automatically
}

QString MidiController::presetExtension() {
    return MIDI_MAPPING_EXTENSION;
}

void MidiController::applyPreset() {

    Controller::applyPreset();  // Handles the engine

    // Only execute this code if this is an output device
    if (m_bIsOutputDevice) {

        if (m_outputs.count()>0) destroyOutputHandlers();

        createOutputHandlers();
        updateAllOutputs();
    }
}

void MidiController::createOutputHandlers() {
    if (m_outputMappings.isEmpty()) return;

    QHashIterator<ConfigKey, MidiOutput> outIt(m_outputMappings);
    while (outIt.hasNext()) {
        outIt.next();

        MidiOutput outputPack = outIt.value();

        QString group = outIt.key().group;
        QString key = outIt.key().item;

        unsigned char status = outputPack.status;
        unsigned char control = outputPack.control;
        unsigned char on = outputPack.on;
        unsigned char off = outputPack.off;
        float min = outputPack.min;
        float max = outputPack.max;

        if (debugging()) {
            qDebug() << QString(
                "Creating output handler for %1,%2 between %3 and %4 to MIDI out: 0x%5 0x%6, on: 0x%7 off: 0x%8")
                    .arg(group, key,
                            QString::number(min), QString::number(max),
                            QString::number(status, 16).toUpper(),
                            QString::number(control, 16).toUpper().rightJustified(2,'0'),
                            QString::number(on, 16).toUpper().rightJustified(2,'0'),
                            QString::number(off, 16).toUpper().rightJustified(2,'0'));
        }

        MidiOutputHandler* moh = new MidiOutputHandler(group, key, this, min, max, status, control, on, off);
        if (!moh->validate()) {
            QString errorLog = QString("Invalid MixxxControl: %1, %2")
                                        .arg(group, key).toUtf8();
            if (debugging()) {
                qCritical() << errorLog;
            }
            else {
                qWarning() << errorLog;
                ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
                props->setType(DLG_WARNING);
                props->setTitle(tr("MixxxControl not found"));
                props->setText(QString(tr("The MixxxControl '%1, %2' specified in the "
                                "loaded mapping is invalid."))
                                .arg(group, key));
                props->setInfoText(QString(tr("The MIDI output message 0x%1 0x%2 will not be bound."
                                "\n(Click Show Details for hints.)"))
                                   .arg(QString::number(status, 16).toUpper(),
                                        QString::number(control, 16).toUpper().rightJustified(2,'0')));
                QString detailsText = QString(tr("* Check to see that the "
                    "MixxxControl name is spelled correctly in the mapping "
                    "file (.xml)\n"));
                detailsText += QString(tr("* Make sure the MixxxControl you're trying to use actually exists."
                    " Visit this wiki page for a complete list:"));
                detailsText += QString("\nhttp://mixxx.org/wiki/doku.php/midi_controller_mapping_file_format#ui_midi_controls_and_names");
                props->setDetails(detailsText);

                ErrorDialogHandler::instance()->requestErrorDialog(props);
            }
            delete moh;
            continue;
        }
        
        m_outputs.append(moh);
    }
}

void MidiController::updateAllOutputs() {
    for (int i = m_outputs.count()-1; i >= 0; --i) {
        m_outputs.at(i)->update();
    }
}

void MidiController::destroyOutputHandlers() {
    for (int i = m_outputs.count()-1; i >= 0; --i) {
        delete m_outputs.takeAt(i);
    }
}

void MidiController::receive(unsigned char status, unsigned char control,
                             unsigned char value) {

    unsigned char channel = status & 0x0F;
    unsigned char opCode = status & 0xF0;
    if (opCode >= 0xF0) opCode = status;
    
    QString message;
    bool twoBytes = true;

    switch (opCode) {
        case MIDI_PITCH_BEND:
            twoBytes = false;
            message = QString("MIDI status 0x%1: pitch bend ch %2, value 0x%3")
                 .arg(QString::number(status, 16).toUpper(),
                      QString::number(channel+1, 10),
                      QString::number((value << 7) | control, 16).toUpper().rightJustified(4,'0'));
            break;
        case MIDI_SONG_POS:
            twoBytes = false;
            message = QString("MIDI status 0x%1: song position 0x%2")
                 .arg(QString::number(status, 16).toUpper(),
                      QString::number((value << 7) | control, 16).toUpper().rightJustified(4,'0'));
            break;
        case MIDI_PROGRAM_CH:
        case MIDI_CH_AFTERTOUCH:
            twoBytes = false;
            message = QString("MIDI status 0x%1 (ch %2, opcode 0x%3), value 0x%4")
                 .arg(QString::number(status, 16).toUpper(),
                      QString::number(channel+1, 10),
                      QString::number((status & 255)>>4, 16).toUpper(),
                      QString::number(control, 16).toUpper().rightJustified(2,'0'));
            break;
        case MIDI_SONG:
            message = QString("MIDI status 0x%1: select song #%2")
                 .arg(QString::number(status, 16).toUpper(),
                      QString::number(control+1, 10));
            break;
        case MIDI_NOTE_OFF:
        case MIDI_NOTE_ON:
        case MIDI_AFTERTOUCH:
        case MIDI_CC:
            message = QString("MIDI status 0x%1 (ch %2, opcode 0x%3), ctrl 0x%4, val 0x%5")
                 .arg(QString::number(status, 16).toUpper(),
                      QString::number(channel+1, 10),
                      QString::number((status & 255)>>4, 16).toUpper(),
                      QString::number(control, 16).toUpper().rightJustified(2,'0'),
                      QString::number(value, 16).toUpper().rightJustified(2,'0'));
            break;
        default:
            twoBytes = false;
            message = QString("MIDI status 0x%1")
                 .arg(QString::number(status, 16).toUpper());
            break;
    }

    if (debugging()) qDebug() << message;

//     if (m_bReceiveInhibit) return;

    QPair<ConfigKey, MidiOptions> controlOptions;

    MidiKey mappingKey;
    mappingKey.status = status;

    // When it's part of the message, include it
    if (twoBytes) mappingKey.control = control;
    else {
        // Signifies that the second byte is part of the payload, default
        mappingKey.control = 0xFF;
    }
    
    if (m_bMidiLearn) {
        emit(midiEvent(mappingKey));
        return; // Don't process midi messages further when MIDI learning
    }
    // If no control is bound to this MIDI message, return
    if (!m_mappings.contains(mappingKey.key)) return;
    controlOptions = m_mappings.value(mappingKey.key);

    ConfigKey ckey = controlOptions.first;
    MidiOptions options = controlOptions.second;

    if (options.script) {
        if (m_pEngine == NULL) return;
        
        QScriptValueList args;
        args << QScriptValue(status & 0x0F);
        args << QScriptValue(control);
        args << QScriptValue(value);
        args << QScriptValue(status);
        args << QScriptValue(ckey.group);
        
        m_pEngine->execute(ckey.item, args);
        return;
    }

    ControlObject * p = ControlObject::getControl(ckey);

    if (p) //Only pass values on to valid ControlObjects.
    {
        double currMixxxControlValue = p->GetMidiValue();

        double newValue = value;

//         qDebug() << "MIDI Options" << QString::number(options.all, 2).rightJustified(16,'0');

        // compute 14-bit number for pitch bend messages
        if (opCode == MIDI_PITCH_BEND) {
            unsigned int ivalue;
            ivalue = (value << 7) | control;

            currMixxxControlValue = p->get();

            // Range is 0x0000..0x3FFF center @ 0x2000, i.e. 0..16383 center @ 8192
            newValue = (ivalue-8192)/8191;
            // computeValue not done on pitch messages because it all assumes 7-bit numbers
        }
        else newValue = computeValue(options, currMixxxControlValue, value);

        if (options.soft_takeover) {
            m_st.enable(ckey.group,ckey.item);  // This is the only place to enable it if it isn't already.
            if (m_st.ignore(ckey.group,ckey.item,newValue,true)) return;
        }

        ControlObject::sync();
        
        p->queueFromMidi(static_cast<MidiOpCode>(opCode), newValue);
    }
    return;
}

double MidiController::computeValue(MidiOptions options, double _prevmidivalue, double _newmidivalue) {
    
    double tempval = 0.;
    double diff = 0.;

    if (options.all == 0) return _newmidivalue;

    if (options.invert) return 127. - _newmidivalue;

    if (options.rot64 || options.rot64_inv) {
        tempval = _prevmidivalue;
        diff = _newmidivalue - 64.;
        if (diff == -1 || diff == 1)
            diff /= 16;
        else
            diff += (diff > 0 ? -1 : +1);
        if (options.rot64)
            tempval += diff;
        else
            tempval -= diff;
        return (tempval < 0. ? 0. : (tempval > 127. ? 127.0 : tempval));
    }
    
    if (options.rot64_fast) {
        tempval = _prevmidivalue;
        diff = _newmidivalue - 64.;
        diff *= 1.5;
        tempval += diff;
        return (tempval < 0. ? 0. : (tempval > 127. ? 127.0 : tempval));
    }
    
    if (options.diff) {
        //Interpret 7-bit signed value using two's compliment.
        if (_newmidivalue >= 64.)
            _newmidivalue = _newmidivalue - 128.;
        //Apply sensitivity to signed value. FIXME
       // if(sensitivity > 0)
        //    _newmidivalue = _newmidivalue * ((double)sensitivity / 50.);
        //Apply new value to current value.
        _newmidivalue = _prevmidivalue + _newmidivalue;
    }
    
    if (options.selectknob) {
        //Interpret 7-bit signed value using two's compliment.
        if (_newmidivalue >= 64.)
            _newmidivalue = _newmidivalue - 128.;
        //Apply sensitivity to signed value. FIXME
        //if(sensitivity > 0)
        //    _newmidivalue = _newmidivalue * ((double)sensitivity / 50.);
        //Since this is a selection knob, we do not want to inherit previous values.
    }
    
    if (options.button) { _newmidivalue = (_newmidivalue != 0); }
    
    if (options.sw) { _newmidivalue = 1; }
    
    if (options.spread64) {
//         qDebug() << "MIDI_OPT_SPREAD64";
        // BJW: Spread64: Distance away from centre point (aka "relative CC")
        // Uses a similar non-linear scaling formula as ControlTTRotary::getValueFromWidget()
        // but with added sensitivity adjustment. This formula is still experimental.

        _newmidivalue = _newmidivalue - 64.;
        //FIXME
        //double distance = _newmidivalue - 64.;
        // _newmidivalue = distance * distance * sensitivity / 50000.;
        //if (distance < 0.)
        //    _newmidivalue = -_newmidivalue;

         //qDebug() << "Spread64: in " << distance << "  out " << _newmidivalue;
    }
    
    if (options.herc_jog) {
        if (_newmidivalue > 64.) { _newmidivalue -= 128.; }
        _newmidivalue += _prevmidivalue;
        //if (_prevmidivalue != 0.0) { qDebug() << "AAAAAAAAAAAA" << _prevmidivalue; }
    }

    return _newmidivalue;
}

void MidiController::receive(const unsigned char data[], unsigned int length) {

    QString message = m_sDeviceName+": [";
    for (uint i = 0; i < length; ++i) {
        message += QString("%1%2").arg(
            QString("%1").arg(data[i], 2, 16, QChar('0')).toUpper(),
            QString("%1").arg((i < (length-1)) ? ' ' : ']'));
    }

    if (debugging()) qDebug()<< message;

//     if (m_bReceiveInhibit) return;

    QPair<ConfigKey, MidiOptions> control;

    MidiKey mappingKey;
    mappingKey.status = data[0];
    // Signifies that the second byte is part of the payload, default
    mappingKey.control = 0xFF;
    
    if (m_bMidiLearn) {
        emit(midiEvent(mappingKey));
        return; // Don't process midi messages further when MIDI learning
    }
    // If no control is bound to this MIDI status, return
    if (!m_mappings.contains(mappingKey.key)) return;
    control = m_mappings.value(mappingKey.key);
    
    ConfigKey ckey = control.first;
    MidiOptions options = control.second;

    // Custom script handler
    if (options.script) {
        if (m_pEngine == NULL) return;
//         // Up-cast to ControllerEngine since this version of execute() is not in MCE
//         //  (polymorphism doesn't work across class boundaries)
//         ControllerEngine *pEngine = m_pEngine;
//         if (!pEngine->execute(ckey.item, data, length)) {
        if (!m_pEngine->execute(ckey.item, data, length)) {
            qDebug() << "MidiController: Invalid script function" << ckey.item;
        }
        return;
    }

    qWarning() << "MidiController: No script function specified for" << message;
}

void MidiController::sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2) {
    unsigned int word = (((unsigned int)byte2) << 16) |
    (((unsigned int)byte1) << 8) | status;
    send(word);
}

void MidiController::send(unsigned int word) {
    Q_UNUSED(word);
    qDebug() << "MIDI short message sending not yet implemented for this API or platform";
}

QDomElement MidiController::loadPreset(QDomElement root, bool forceLoad) {

    if (root.isNull()) return root;

    m_bindings = root;

    // Superclass handles script files
    QDomElement controller = Controller::loadPreset(root,forceLoad);

    if (!controller.isNull()) {

        m_mappings.clear();
        m_outputMappings.clear();

        /*
        // We actually need to load any script code now to verify function
        //  names against those in the XML
        qDebug() << "Supposed to load" << m_scriptFileNames.size() << "script files here";
        QStringList scriptFunctions;
        if (m_pEngine != NULL) {
            m_pEngine->loadScriptFiles(m_scriptFileNames);
            scriptFunctions = m_pEngine->getScriptFunctions();
        }
        else qWarning() << "No engine!";
        */

        QDomElement control = controller.firstChildElement("controls").firstChildElement("control");

        //Iterate through each <control> block in the XML
        while (!control.isNull()) {

            //Unserialize these objects from the XML
//             MidiMessage midiMessage(control);

            QString strMidiStatus = control.firstChildElement("status").text();
            QString midiNo = control.firstChildElement("midino").text();

            bool ok = false;

            //Use QString with toInt base of 0 to auto convert hex values
            unsigned char midiStatusByte = strMidiStatus.toInt(&ok, 0);
            if (!ok) midiStatusByte = 0x00;

            unsigned char midiControl = midiNo.toInt(&ok, 0);
            if (!ok) midiControl = 0x00;

//             MixxxControl mixxxControl(control);
            QDomElement groupNode = control.firstChildElement("group");
            QDomElement keyNode = control.firstChildElement("key");
            QDomElement descriptionNode = control.firstChildElement("description");

            QString controlGroup = groupNode.text();
            QString controlKey = keyNode.text();
            QString controlDescription = descriptionNode.text();

            // Get options
            QDomElement optionsNode = control.firstChildElement("options").firstChildElement();

            MidiOptions options;
            options.all = 0;

            QString strMidiOption;
            while (!optionsNode.isNull()) {
                strMidiOption = optionsNode.nodeName().toLower();

                // "normal" is no options
                if (strMidiOption == "invert")   options.invert = true;
                if (strMidiOption == "rot64")    options.rot64 = true;
                if (strMidiOption == "rot64inv") options.rot64_inv = true;
                if (strMidiOption == "rot64fast")options.rot64_fast = true;
                if (strMidiOption == "diff")     options.diff = true;
                if (strMidiOption == "button")   options.button = true;
                if (strMidiOption == "switch")   options.sw = true;
                if (strMidiOption == "hercjog")  options.herc_jog = true;
                if (strMidiOption == "spread64") options.spread64 = true;
                if (strMidiOption == "selectknob")options.selectknob = true;
                if (strMidiOption == "soft-takeover") options.soft_takeover = true;
                if (strMidiOption == "script-binding") options.script = true;

                optionsNode = optionsNode.nextSiblingElement();
            }

            /*
            // Verify script functions are loaded
            if (options.script && !scriptFunctions.isEmpty() && scriptFunctions.indexOf(controlKey)==-1) {

                QString status = QString("0x%1").arg(midiStatusByte, 2, 16, QChar('0')).toUpper();
                QString byte2 = QString("0x%1").arg(midiControl, 2, 16, QChar('0')).toUpper();

                // If status is MIDI pitch, the 2nd byte is part of the payload so don't display it
                if ((midiStatusByte & 0xF0) == MIDI_PITCH_BEND) byte2 = "";

                QString errorLog = QString("MIDI script function \"%1\" not found. "
                                           "(Mapped to MIDI message %2 %3)")
                                           .arg(controlKey, status, byte2);

                if (m_bIsOutputDevice && debugging()) {
                        qCritical() << errorLog;
                }
                else {
                    qWarning() << errorLog;
                    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
                    props->setType(DLG_WARNING);
                    props->setTitle(tr("MIDI script function not found"));
                    props->setText(QString(tr("The MIDI script function '%1' was not "
                                    "found in loaded scripts."))
                                    .arg(controlKey));
                    props->setInfoText(QString(tr("The MIDI message %1 %2 will not be bound."
                                    "\n(Click Show Details for hints.)"))
                                       .arg(status, byte2));
                    QString detailsText = QString(tr("* Check to see that the "
                        "function name is spelled correctly in the mapping "
                        "file (.xml) and script file (.js)\n"));
                    detailsText += QString(tr("* Check to see that the script "
                        "file name (.js) is spelled correctly in the mapping "
                        "file (.xml)"));
                    props->setDetails(detailsText);

                    ErrorDialogHandler::instance()->requestErrorDialog(props);
                }
            } else
            */
            {
                // Add the static mapping
                QPair<ConfigKey, MidiOptions> target;
                target.first = ConfigKey(controlGroup, controlKey);
                target.second = options;
                
                unsigned char opCode = midiStatusByte & 0xF0;
                if (opCode >= 0xF0) opCode = midiStatusByte;

                bool twoBytes = true;
                switch (opCode) {
                    case MIDI_SONG:
                    case MIDI_NOTE_OFF:
                    case MIDI_NOTE_ON:
                    case MIDI_AFTERTOUCH:
                    case MIDI_CC:
                        break;
                    case MIDI_PITCH_BEND:
                    case MIDI_SONG_POS:
                    case MIDI_PROGRAM_CH:
                    case MIDI_CH_AFTERTOUCH:
                    default:
                        twoBytes = false;
                        break;
                }

                MidiKey key;
                key.status = midiStatusByte;
                
                if (twoBytes) key.control = midiControl;
                else {
                    // Signifies that the second byte is part of the payload, default
                    key.control = 0xFF;
                }
/*                qDebug() << "New mapping:" << QString::number(key.key, 16).toUpper()
                          << QString::number(key.status, 16).toUpper()
                          << QString::number(key.control, 16).toUpper()
                          << target.first.group << target.first.item;
*/
                m_mappings.insert(key.key, target);
                // Notify the GUI and anyone else who cares
//                 emit(newMapping());  // TODO
            }
            control = control.nextSiblingElement("control");
        }

        qDebug() << "MidiController: Input mapping parsing complete.";

        // Parse static output mappings

        QDomElement output = controller.firstChildElement("outputs").firstChildElement("output");

        //Iterate through each <control> block in the XML
        while (!output.isNull()) {
            //Unserialize the ConfigKey components from the XML
            QDomElement groupNode = output.firstChildElement("group");
            QDomElement keyNode = output.firstChildElement("key");
            
            QString controlGroup = groupNode.text();
            QString controlKey = keyNode.text();
            
            // Unserialize output message from the XML
            MidiOutput outputMessage;

            QString midiStatus = output.firstChildElement("status").text();
            QString midiNo = output.firstChildElement("midino").text();
            QString midiOn = output.firstChildElement("on").text();
            QString midiOff = output.firstChildElement("off").text();

            bool ok = false;

            //Use QString with toInt base of 0 to auto convert hex values
            outputMessage.status = midiStatus.toInt(&ok, 0);
            if (!ok) outputMessage.status = 0x00;

            outputMessage.control = midiNo.toInt(&ok, 0);
            if (!ok) outputMessage.control = 0x00;

            outputMessage.on = midiOn.toInt(&ok, 0);
            if (!ok) outputMessage.on = DEFAULT_OUTPUT_ON;

            outputMessage.off = midiOff.toInt(&ok, 0);
            if (!ok) outputMessage.off = DEFAULT_OUTPUT_OFF;

            QDomElement minNode = output.firstChildElement("minimum");
            QDomElement maxNode = output.firstChildElement("maximum");

            ok = false;
            if (!minNode.isNull()) {
                outputMessage.min = minNode.text().toFloat(&ok);
            } else {
                ok = false;
            }

            if (!ok) //If not a float, or node wasn't defined
                outputMessage.min = DEFAULT_OUTPUT_MIN;

            if (!maxNode.isNull()) {
                outputMessage.max = maxNode.text().toFloat(&ok);
            } else {
                ok = false;
            }

            if (!ok) //If not a float, or node wasn't defined
                outputMessage.max = DEFAULT_OUTPUT_MAX;

            // END unserialize output
            
//             MixxxControl mixxxControl(output, true);

            // Add the static output mapping.
            /*
            qDebug() << "New output mapping:"
                          << QString::number(outputMessage.status, 16).toUpper()
                          << QString::number(outputMessage.control, 16).toUpper()
                          << QString::number(outputMessage.on, 16).toUpper()
                          << QString::number(outputMessage.off, 16).toUpper()
                          << controlGroup << controlKey;
            */
            // We use insertMulti because certain tricks are done with multiple
            //  entries for the same ConfigKey
            m_outputMappings.insertMulti(ConfigKey(controlGroup, controlKey),outputMessage);
            
//             internalSetOutputMidiMapping(mixxxControl, midiMessage, true);
            /*Old code: m_outputMapping.insert(mixxxControl, midiMessage);
              Reason why this is bad: Don't want to access this directly because the
                                      model doesn't get notified about the update */

            output = output.nextSiblingElement("output");
        }

        qDebug() << "MidiController: Output mapping parsing complete.";
    }

    return m_bindings;
}

QDomDocument MidiController::buildDomElement() {

    // The XML header and script stuff is handled by the superclass
    QDomDocument doc = Controller::buildDomElement();

    QDomElement controller = doc.documentElement().firstChildElement("controller");
    QDomElement controls = doc.createElement("controls");
    controller.appendChild(controls);

    //Iterate over all of the command/control pairs in the input mapping
    QHashIterator<uint16_t, QPair<ConfigKey, MidiOptions> > it(m_mappings);
    while (it.hasNext()) {
        it.next();

        QDomElement controlNode = doc.createElement("control");

        QDomText text;
        QDomDocument nodeMaker;
        QDomElement tagNode;

        QPair<ConfigKey, MidiOptions> target = it.value();
        MidiKey package;
        package.key = it.key();

//         qDebug() << QString::number(package.key, 16).toUpper()
//                  << QString::number(package.status, 16).toUpper()
//                  << QString::number(package.control, 16).toUpper()
//                  << target.first.group << target.first.item;
        mappingToXML(controlNode, target.first.group, target.first.item,
                     package.status, package.control);

        //Midi options
        QDomElement optionsNode = nodeMaker.createElement("options");
        MidiOptions options = it.value().second;

        // "normal" is no options
        if (options.all == 0) {
            QDomElement singleOption = nodeMaker.createElement("normal");
            optionsNode.appendChild(singleOption);
        }
        else {
            if (options.invert) {
                QDomElement singleOption = nodeMaker.createElement("invert");
                optionsNode.appendChild(singleOption);
            }
            if (options.rot64) {
                QDomElement singleOption = nodeMaker.createElement("rot64");
                optionsNode.appendChild(singleOption);
            }
            if (options.rot64_inv) {
                QDomElement singleOption = nodeMaker.createElement("rot64inv");
                optionsNode.appendChild(singleOption);
            }
            if (options.rot64_fast) {
                QDomElement singleOption = nodeMaker.createElement("rot64fast");
                optionsNode.appendChild(singleOption);
            }
            if (options.diff) {
                QDomElement singleOption = nodeMaker.createElement("diff");
                optionsNode.appendChild(singleOption);
            }
            if (options.button) {
                QDomElement singleOption = nodeMaker.createElement("button");
                optionsNode.appendChild(singleOption);
            }
            if (options.sw) {
                QDomElement singleOption = nodeMaker.createElement("switch");
                optionsNode.appendChild(singleOption);
            }
            if (options.herc_jog) {
                QDomElement singleOption = nodeMaker.createElement("hercjog");
                optionsNode.appendChild(singleOption);
            }
            if (options.spread64) {
                QDomElement singleOption = nodeMaker.createElement("spread64");
                optionsNode.appendChild(singleOption);
            }
            if (options.selectknob) {
                QDomElement singleOption = nodeMaker.createElement("selectknob");
                optionsNode.appendChild(singleOption);
            }
            if (options.soft_takeover) {
                QDomElement singleOption = nodeMaker.createElement("soft-takeover");
                optionsNode.appendChild(singleOption);
            }
            if (options.script) {
                QDomElement singleOption = nodeMaker.createElement("script-binding");
                optionsNode.appendChild(singleOption);
            }
        }

        controlNode.appendChild(optionsNode);

        //Add the control node we just created to the XML document in the proper spot
        controls.appendChild(controlNode);
    }

    QDomElement outputs = doc.createElement("outputs");
    controller.appendChild(outputs);

    //Iterate over all of the command/control pairs in the OUTPUT mapping
    QHashIterator<ConfigKey, MidiOutput> outIt(m_outputMappings);
    while (outIt.hasNext()) {
        outIt.next();

        QDomElement outputNode = doc.createElement("output");
        MidiOutput outputPack = outIt.value();

        mappingToXML(outputNode, outIt.key().group, outIt.key().item, outputPack.status, outputPack.control);
        outputMappingToXML(outputNode, outputPack.on, outputPack.off, outputPack.max, outputPack.min);

        //Add the control node we just created to the XML document in the proper spot
        outputs.appendChild(outputNode);
    }

    m_bindings = doc.documentElement();
    return doc;
}

void MidiController::mappingToXML(QDomElement& parentNode, QString group,
                                  QString item, unsigned char status, unsigned char control) const
{
    QDomText text;
    QDomDocument nodeMaker;
    QDomElement tagNode;

    //Control object group
    tagNode = nodeMaker.createElement("group");
    text = nodeMaker.createTextNode(group);
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);
    
    //Control object name
    tagNode = nodeMaker.createElement("key"); //WTF worst name ever
    text = nodeMaker.createTextNode(item);
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);
    
    //MIDI status byte
    tagNode = nodeMaker.createElement("status");
    text = nodeMaker.createTextNode(QString("0x%1").arg(QString::number(status, 16).toUpper().rightJustified(2,'0')));
    tagNode.appendChild(text);
    parentNode.appendChild(tagNode);
    
    if (control != 0xFF) {
        //MIDI control number
        tagNode = nodeMaker.createElement("midino");
        text = nodeMaker.createTextNode(QString("0x%1").arg(QString::number(control, 16).toUpper().rightJustified(2,'0')));
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);
    }
}

void MidiController::outputMappingToXML(QDomElement& parentNode, unsigned char on,
                                        unsigned char off, float max, float min) const
{
    QDomText text;
    QDomDocument nodeMaker;
    QDomElement tagNode;
    
    // Third MIDI byte for turning on the LED
    if (on != DEFAULT_OUTPUT_ON) {
        tagNode = nodeMaker.createElement("on");
        text = nodeMaker.createTextNode(QString("0x%1").arg(QString::number(on, 16).toUpper().rightJustified(2,'0')));
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);
    }
    
    // Third MIDI byte for turning off the LED
    if (off != DEFAULT_OUTPUT_OFF) {
        tagNode = nodeMaker.createElement("off");
        text = nodeMaker.createTextNode(QString("0x%1").arg(QString::number(off, 16).toUpper().rightJustified(2,'0')));
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);
    }

    // Upper value, above which the 'off' value is sent
    //  (We don't bother writing it if it's the default value.)
    if (max != DEFAULT_OUTPUT_MAX) {
        tagNode = nodeMaker.createElement("maximum");
        QString value;
        value.setNum(max);
        text = nodeMaker.createTextNode(value);
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);
    }

    // Lower value, below which the 'off' value is sent
    //  (We don't bother writing it if it's the default value.)
    if (min != DEFAULT_OUTPUT_MIN) {
        tagNode = nodeMaker.createElement("minimum");
        QString value;
        value.setNum(min);
        text = nodeMaker.createTextNode(value);
        tagNode.appendChild(text);
        parentNode.appendChild(tagNode);
    }
}