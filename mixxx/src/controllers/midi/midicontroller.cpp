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
#include "controlobject.h"
#include "errordialoghandler.h"

#include "../defs_controllers.h"   // For MIDI_MAPPING_EXTENSION

MidiController::MidiController() : Controller() {
    m_bMidiLearn = false;
}

MidiController::~MidiController(){
    destroyOutputHandlers();
//     close(); // I wish I could put this here to enforce it automatically
}

QString MidiController::defaultPreset() {
    return PRESETS_PATH.append(m_sDeviceName.right(m_sDeviceName.size()
            -m_sDeviceName.indexOf(" ")-1).replace(" ", "_")
            + presetExtension());
}

QString MidiController::presetExtension() {
    return MIDI_MAPPING_EXTENSION;
}

void MidiController::visit(const MidiControllerPreset* preset) {
    m_preset = *preset;
}

void MidiController::visit(const HidControllerPreset* preset) {
    qDebug() << "ERROR: Attempting to load an HidControllerPreset to a MidiController!";
    // TODO(XXX): throw a hissy fit.
}

bool MidiController::savePreset(const QString fileName) const {
    MidiControllerPresetFileHandler handler;
    return handler.save(m_preset, getName(), fileName);
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
    if (m_preset.outputMappings.isEmpty()) return;

    QHashIterator<ConfigKey, MidiOutput> outIt(m_preset.outputMappings);
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
    if (!m_preset.mappings.contains(mappingKey.key)) return;
    controlOptions = m_preset.mappings.value(mappingKey.key);

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

void MidiController::receive(QByteArray data) {

    int length = data.size();
    QString message = m_sDeviceName+": [";
    for (int i = 0; i < length; ++i) {
        message += QString("%1%2").arg(
            QString("%1").arg((unsigned char)(data.at(i)), 2, 16, QChar('0')).toUpper(),
            QString("%1").arg((i < (length-1)) ? ' ' : ']'));
    }

    if (debugging()) qDebug()<< message;

//     if (m_bReceiveInhibit) return;

    QPair<ConfigKey, MidiOptions> control;

    MidiKey mappingKey;
    mappingKey.status = data.at(0);
    // Signifies that the second byte is part of the payload, default
    mappingKey.control = 0xFF;

    if (m_bMidiLearn) {
        emit(midiEvent(mappingKey));
        return; // Don't process midi messages further when MIDI learning
    }
    // If no control is bound to this MIDI status, return
    if (!m_preset.mappings.contains(mappingKey.key)) return;
    control = m_preset.mappings.value(mappingKey.key);

    ConfigKey ckey = control.first;
    MidiOptions options = control.second;

    // Custom script handler
    if (options.script) {
        if (m_pEngine == NULL) return;
//         // Up-cast to ControllerEngine since this version of execute() is not in MCE
//         //  (polymorphism doesn't work across class boundaries)
//         ControllerEngine *pEngine = m_pEngine;
//         if (!pEngine->execute(ckey.item, data, length)) {
        if (!m_pEngine->execute(ckey.item, data)) {
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
