/**
 * @file midicontroller.cpp
 * @author Sean Pappalardo spappalardo@mixxx.org
 * @date Tue 7 Feb 2012
 * @brief MIDI Controller base class
 *
 */

#include "controllers/midi/midicontroller.h"

#include "controllers/midi/midiutils.h"
#include "controllers/defs_controllers.h"
#include "controllers/controllerdebug.h"
#include "controlobject.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"
#include "util/math.h"

MidiController::MidiController()
        : Controller() {
    setDeviceCategory(tr("MIDI Controller"));
}

MidiController::~MidiController() {
    destroyOutputHandlers();
    // Don't close the device here. Sub-classes should close the device in their
    // destructors.
}

QString MidiController::presetExtension() {
    return MIDI_PRESET_EXTENSION;
}

void MidiController::visit(const MidiControllerPreset* preset) {
    m_preset = *preset;
    emit(presetLoaded(getPreset()));
}

int MidiController::close() {
    destroyOutputHandlers();
    return 0;
}

void MidiController::visit(const HidControllerPreset* preset) {
    Q_UNUSED(preset);
    qWarning() << "ERROR: Attempting to load an HidControllerPreset to a MidiController!";
    // TODO(XXX): throw a hissy fit.
}

bool MidiController::matchPreset(const PresetInfo& preset) {
    // Product info mapping not implemented for MIDI devices yet
    Q_UNUSED(preset);
    return false;
}

bool MidiController::savePreset(const QString fileName) const {
    MidiControllerPresetFileHandler handler;
    return handler.save(m_preset, getName(), fileName);
}

bool MidiController::applyPreset(QList<QString> scriptPaths, bool initializeScripts) {
    // Handles the engine
    bool result = Controller::applyPreset(scriptPaths, initializeScripts);

    // Only execute this code if this is an output device
    if (isOutputDevice()) {
        if (m_outputs.count() > 0) {
            destroyOutputHandlers();
        }
        createOutputHandlers();
        updateAllOutputs();
    }
    return result;
}

void MidiController::createOutputHandlers() {
    if (m_preset.outputMappings.isEmpty()) {
        return;
    }

    QHashIterator<ConfigKey, MidiOutputMapping> outIt(m_preset.outputMappings);
    QStringList failures;
    while (outIt.hasNext()) {
        outIt.next();

        const MidiOutputMapping& mapping = outIt.value();

        QString group = mapping.controlKey.group;
        QString key = mapping.controlKey.item;

        unsigned char status = mapping.output.status;
        unsigned char control = mapping.output.control;
        unsigned char on = mapping.output.on;
        unsigned char off = mapping.output.off;
        double min = mapping.output.min;
        double max = mapping.output.max;

        controllerDebug(QString(
                "Creating output handler for %1,%2 between %3 and %4 to MIDI out: 0x%5 0x%6, on: 0x%7 off: 0x%8")
                        .arg(group, key,
                                QString::number(min), QString::number(max),
                                QString::number(status, 16).toUpper(),
                                QString::number(control, 16).toUpper().rightJustified(2,'0'),
                                QString::number(on, 16).toUpper().rightJustified(2,'0'),
                                QString::number(off, 16).toUpper().rightJustified(2,'0')));

        MidiOutputHandler* moh = new MidiOutputHandler(this, mapping);
        if (!moh->validate()) {
            QString errorLog =
                QString("MIDI output message 0x%1 0x%2 has invalid MixxxControl %3, %4")
                        .arg(QString::number(status, 16).toUpper(),
                             QString::number(control, 16).toUpper().rightJustified(2,'0'))
                        .arg(group, key).toUtf8();
            qWarning() << errorLog;

            int deckNum = 0;
            if (ControllerDebug::enabled()) {
                failures.append(errorLog);
            } else if (PlayerManager::isDeckGroup(group, &deckNum)) {
                int numDecks = PlayerManager::numDecks();
                if (deckNum <= numDecks) {
                    failures.append(errorLog);
                }
            }

            delete moh;
            continue;
        }
        m_outputs.append(moh);
    }

    if (!failures.isEmpty()) {
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(tr("MixxxControl(s) not found"));
        props->setText(tr("One or more MixxxControls specified in the "
                          "outputs section of the loaded preset were invalid."));
        props->setInfoText(tr("Some LEDs or other feedback may not work correctly."));
        QString detailsText = tr("* Check to see that the MixxxControl "
                                 "names are spelled correctly in the mapping "
                                 "file (.xml)\n");
        detailsText += tr("* Make sure the MixxxControls in question actually exist."
                          " Visit this wiki page for a complete list: ");
        detailsText += "http://mixxx.org/wiki/doku.php/mixxxcontrols\n\n";
        detailsText += failures.join("\n");
        props->setDetails(detailsText);
        ErrorDialogHandler::instance()->requestErrorDialog(props);
    }
}

void MidiController::updateAllOutputs() {
    foreach (MidiOutputHandler* pOutput, m_outputs) {
        pOutput->update();
    }
}

void MidiController::destroyOutputHandlers() {
    while (m_outputs.size() > 0) {
        delete m_outputs.takeLast();
    }
}

QString formatMidiMessage(const QString& controllerName,
                          unsigned char status, unsigned char control,
                          unsigned char value, unsigned char channel,
                          unsigned char opCode, mixxx::Duration timestamp) {
    switch (opCode) {
        case MIDI_PITCH_BEND:
            return QString("%1: t:%2 status 0x%3: pitch bend ch %4, value 0x%5")
                    .arg(controllerName, timestamp.formatMillisWithUnit(),
                         QString::number(status, 16).toUpper(),
                         QString::number(channel+1, 10),
                         QString::number((value << 7) | control, 16).toUpper().rightJustified(4,'0'));
        case MIDI_SONG_POS:
            return QString("%1: t:%5 status 0x%3: song position 0x%4")
                    .arg(controllerName, timestamp.formatMillisWithUnit(),
                         QString::number(status, 16).toUpper(),
                         QString::number((value << 7) | control, 16).toUpper().rightJustified(4,'0'));
        case MIDI_PROGRAM_CH:
        case MIDI_CH_AFTERTOUCH:
            return QString("%1: t:%2 status 0x%3 (ch %4, opcode 0x%5), value 0x%6")
                    .arg(controllerName, timestamp.formatMillisWithUnit(),
                         QString::number(status, 16).toUpper(),
                         QString::number(channel+1, 10),
                         QString::number((status & 255)>>4, 16).toUpper(),
                         QString::number(control, 16).toUpper().rightJustified(2,'0'));
        case MIDI_SONG:
            return QString("%1: t:%2 status 0x%3: select song #%4")
                    .arg(controllerName, timestamp.formatMillisWithUnit(),
                         QString::number(status, 16).toUpper(),
                         QString::number(control+1, 10));
        case MIDI_NOTE_OFF:
        case MIDI_NOTE_ON:
        case MIDI_AFTERTOUCH:
        case MIDI_CC:
            return QString("%1: t:%2 status 0x%3 (ch %4, opcode 0x%5), ctrl 0x%6, val 0x%7")
                    .arg(controllerName, timestamp.formatMillisWithUnit(),
                         QString::number(status, 16).toUpper(),
                         QString::number(channel+1, 10),
                         QString::number((status & 255)>>4, 16).toUpper(),
                         QString::number(control, 16).toUpper().rightJustified(2,'0'),
                         QString::number(value, 16).toUpper().rightJustified(2,'0'));
        default:
            return QString("%1: t:%2 status 0x%3")
                    .arg(controllerName, timestamp.formatMillisWithUnit(),
                         QString::number(status, 16).toUpper());
    }
}

void MidiController::learnTemporaryInputMappings(const MidiInputMappings& mappings) {
    foreach (const MidiInputMapping& mapping, mappings) {
        m_temporaryInputMappings.insert(mapping.key.key, mapping);

        unsigned char opCode = MidiUtils::opCodeFromStatus(mapping.key.status);
        bool twoBytes = MidiUtils::isMessageTwoBytes(opCode);
        QString message = twoBytes ? QString("0x%1 0x%2")
                .arg(QString::number(mapping.key.status, 16).toUpper(),
                     QString::number(mapping.key.control, 16).toUpper()
                     .rightJustified(2,'0')) :
                QString("0x%1")
                .arg(QString::number(mapping.key.status, 16).toUpper());
        qDebug() << "Set mapping for" << message << "to"
                 << mapping.control.group << mapping.control.item;
    }
}

void MidiController::clearTemporaryInputMappings() {
    m_temporaryInputMappings.clear();
}

void MidiController::commitTemporaryInputMappings() {
    // We want to replace duplicates that exist in m_preset but allow duplicates
    // in m_temporaryInputMappings. To do this, we first remove every key in
    // m_temporaryInputMappings from m_preset.inputMappings.
    for (QHash<uint16_t, MidiInputMapping>::const_iterator it =
                 m_temporaryInputMappings.begin();
         it != m_temporaryInputMappings.end(); ++it) {
        m_preset.inputMappings.remove(it.key());
    }

    // Now, we can just use unite since we manually removed the duplicates in
    // the original set.
    m_preset.inputMappings.unite(m_temporaryInputMappings);
    m_temporaryInputMappings.clear();
}

void MidiController::receive(unsigned char status, unsigned char control,
                             unsigned char value, mixxx::Duration timestamp) {
    unsigned char channel = MidiUtils::channelFromStatus(status);
    unsigned char opCode = MidiUtils::opCodeFromStatus(status);

    controllerDebug(formatMidiMessage(getName(), status, control, value,
                                      channel, opCode, timestamp));
    MidiKey mappingKey(status, control);

    if (isLearning()) {
        emit(messageReceived(status, control, value));

        QHash<uint16_t, MidiInputMapping>::const_iterator it =
                m_temporaryInputMappings.find(mappingKey.key);
        if (it != m_temporaryInputMappings.end()) {
            for (; it != m_temporaryInputMappings.end() && it.key() == mappingKey.key; ++it) {
                processInputMapping(it.value(), status, control, value, timestamp);
            }
            return;
        }
    }

    QHash<uint16_t, MidiInputMapping>::const_iterator it =
            m_preset.inputMappings.find(mappingKey.key);
    for (; it != m_preset.inputMappings.end() && it.key() == mappingKey.key; ++it) {
        processInputMapping(it.value(), status, control, value, timestamp);
    }
}

void MidiController::processInputMapping(const MidiInputMapping& mapping,
                                         unsigned char status,
                                         unsigned char control,
                                         unsigned char value,
                                         mixxx::Duration timestamp) {
    Q_UNUSED(timestamp);
    unsigned char channel = MidiUtils::channelFromStatus(status);
    unsigned char opCode = MidiUtils::opCodeFromStatus(status);

    if (mapping.options.script) {
        ControllerEngine* pEngine = getEngine();
        if (pEngine == NULL) {
            return;
        }

        QScriptValue function = pEngine->resolveFunction(mapping.control.item);
        if (!pEngine->execute(function, channel, control, value, status,
                              mapping.control.group, timestamp)) {
            qDebug() << "MidiController: Invalid script function"
                     << mapping.control.item;
        }
        return;
    }

    // Only pass values on to valid ControlObjects.
    ControlObject* pCO = ControlObject::getControl(mapping.control);
    if (pCO == NULL) {
        return;
    }

    double newValue = value;


    bool mapping_is_14bit = mapping.options.fourteen_bit_msb ||
            mapping.options.fourteen_bit_lsb;
    if (!mapping_is_14bit && !m_fourteen_bit_queued_mappings.isEmpty()) {
        qWarning() << "MidiController was waiting for the MSB/LSB of a 14-bit"
                   << "message but the next message received was not mapped as 14-bit."
                   << "Ignoring the original message.";
        m_fourteen_bit_queued_mappings.clear();
    }

    //qDebug() << "MIDI Options" << QString::number(mapping.options.all, 2).rightJustified(16,'0');

    if (mapping_is_14bit) {
        bool found = false;
        for (QList<QPair<MidiInputMapping, unsigned char> >::iterator it =
                     m_fourteen_bit_queued_mappings.begin();
             it != m_fourteen_bit_queued_mappings.end(); ++it) {
            if (it->first.control == mapping.control) {
                if ((it->first.options.fourteen_bit_lsb && mapping.options.fourteen_bit_lsb) ||
                    (it->first.options.fourteen_bit_msb && mapping.options.fourteen_bit_msb)) {
                    qWarning() << "MidiController: 14-bit MIDI mapping has mis-matched LSB/MSB options."
                               << "Ignoring both messages.";
                    m_fourteen_bit_queued_mappings.erase(it);
                    return;
                }

                int iValue = 0;
                if (mapping.options.fourteen_bit_msb) {
                    iValue = (value << 7) | it->second;
                    // qDebug() << "MSB" << value
                    //          << "LSB" << it->second
                    //          << "Joint:" << iValue;
                } else if (mapping.options.fourteen_bit_lsb) {
                    iValue = (it->second << 7) | value;
                    // qDebug() << "MSB" << it->second
                    //          << "LSB" << value
                    //          << "Joint:" << iValue;
                }

                // NOTE(rryan): The 14-bit message ranges from 0x0000 to
                // 0x3FFF. Dividing by 0x81 maps this onto the range of 0 to
                // 127. However, some controllers map the center to MSB 64
                // (0x40) and LSB 0. Dividing by 128 (0x80) maps 0x2000
                // directly to 0x40. See ControlLinPotmeterBehavior and
                // ControlPotmeterBehavior for more fun of this variety :).
                newValue = static_cast<double>(iValue) / 128.0;
                newValue = math_min(newValue, 127.0);

                // Erase the queued message since we processed it.
                m_fourteen_bit_queued_mappings.erase(it);

                found = true;
                break;
            }
        }
        if (!found) {
            // Queue this mapping and value for processing once we receive the next
            // message.
            m_fourteen_bit_queued_mappings.append(qMakePair(mapping, value));
            return;
        }
    } else if (opCode == MIDI_PITCH_BEND) {
        // compute 14-bit value for pitch bend messages
        int iValue;
        iValue = (value << 7) | control;

        // NOTE(rryan): The 14-bit message ranges from 0x0000 to
        // 0x3FFF. Dividing by 0x81 maps this onto the range of 0 to
        // 127. However, some controllers map the center to MSB 64
        // (0x40) and LSB 0. Dividing by 128 (0x80) maps 0x2000
        // directly to 0x40. See ControlLinPotmeterBehavior and
        // ControlPotmeterBehavior for more fun of this variety :).
        newValue = static_cast<double>(iValue) / 128.0;
        newValue = math_min(newValue, 127.0);
    } else {
        double currControlValue = pCO->getMidiParameter();
        newValue = computeValue(mapping.options, currControlValue, value);
    }

    // ControlPushButton ControlObjects only accept NOTE_ON, so if the midi
    // mapping is <button> we override the Midi 'status' appropriately.
    if (mapping.options.button || mapping.options.sw) {
        opCode = MIDI_NOTE_ON;
    }

    if (mapping.options.soft_takeover) {
        // This is the only place to enable it if it isn't already.
        m_st.enable(pCO);
        if (m_st.ignore(pCO, pCO->getParameterForMidiValue(newValue))) {
            return;
        }
    }
    pCO->setValueFromMidi(static_cast<MidiOpCode>(opCode), newValue);
}

double MidiController::computeValue(MidiOptions options, double _prevmidivalue, double _newmidivalue) {
    double tempval = 0.;
    double diff = 0.;

    if (options.all == 0) {
        return _newmidivalue;
    }

    if (options.invert) {
        return 127. - _newmidivalue;
    }

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

    if (options.button) {
        _newmidivalue = _newmidivalue != 0;
    }

    if (options.sw) {
        _newmidivalue = 1;
    }

    if (options.spread64) {
        //qDebug() << "MIDI_OPT_SPREAD64";
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
        if (_newmidivalue > 64.) {
            _newmidivalue -= 128.;
        }
        _newmidivalue += _prevmidivalue;
        //if (_prevmidivalue != 0.0) { qDebug() << "AAAAAAAAAAAA" << _prevmidivalue; }
    }

    if (options.herc_jog_fast) {
        if (_newmidivalue > 64.) {
            _newmidivalue -= 128.;
        }
        _newmidivalue = _prevmidivalue + (_newmidivalue * 3);
    }

    return _newmidivalue;
}

QString formatSysexMessage(const QString& controllerName, const QByteArray& data,
                           mixxx::Duration timestamp) {
    QString message = QString("%1: t:%2 %3 bytes: [")
            .arg(controllerName).arg(timestamp.formatMillisWithUnit())
            .arg(data.size());
    for (int i = 0; i < data.size(); ++i) {
        message += QString("%1%2").arg(
            QString("%1").arg((unsigned char)(data.at(i)), 2, 16, QChar('0')).toUpper(),
            QString("%1").arg((i < (data.size()-1)) ? ' ' : ']'));
    }
    return message;
}

void MidiController::receive(QByteArray data, mixxx::Duration timestamp) {
    controllerDebug(formatSysexMessage(getName(), data, timestamp));

    MidiKey mappingKey(data.at(0), 0xFF);

    // TODO(rryan): Need to review how MIDI learn works with sysex messages. I
    // don't think this actually does anything useful.
    if (isLearning()) {
        // TODO(rryan): Fake a one value?
        emit(messageReceived(mappingKey.status, mappingKey.control, 0x7F));

        QHash<uint16_t, MidiInputMapping>::const_iterator it =
                m_temporaryInputMappings.find(mappingKey.key);
        if (it != m_temporaryInputMappings.end()) {
            for (; it != m_temporaryInputMappings.end() && it.key() == mappingKey.key; ++it) {
                processInputMapping(it.value(), data, timestamp);
            }
            return;
        }
    }

    QHash<uint16_t, MidiInputMapping>::const_iterator it =
            m_preset.inputMappings.find(mappingKey.key);
    for (; it != m_preset.inputMappings.end() && it.key() == mappingKey.key; ++it) {
        processInputMapping(it.value(), data, timestamp);
    }
}

void MidiController::processInputMapping(const MidiInputMapping& mapping,
                                         const QByteArray& data,
                                         mixxx::Duration timestamp) {
    // Custom script handler
    if (mapping.options.script) {
        ControllerEngine* pEngine = getEngine();
        if (pEngine == NULL) {
            return;
        }
        QScriptValue function = pEngine->resolveFunction(mapping.control.item);
        if (!pEngine->execute(function, data, timestamp)) {
            qDebug() << "MidiController: Invalid script function"
                     << mapping.control.item;
        }
        return;
    }
    qWarning() << "MidiController: No script function specified for"
               << formatSysexMessage(getName(), data, timestamp);
}

void MidiController::sendShortMsg(unsigned char status, unsigned char byte1,
                                  unsigned char byte2) {
    unsigned int word = (((unsigned int)byte2) << 16) |
            (((unsigned int)byte1) << 8) | status;
    sendWord(word);
}
