#include "controllers/midi/midicontroller.h"

#include "control/controlobject.h"
#include "controllers/controllerdebug.h"
#include "controllers/defs_controllers.h"
#include "controllers/midi/midiutils.h"
#include "defs_urls.h"
#include "errordialoghandler.h"
#include "mixer/playermanager.h"
#include "moc_midicontroller.cpp"
#include "util/math.h"
#include "util/screensaver.h"

MidiController::MidiController()
        : Controller() {
    setDeviceCategory(tr("MIDI Controller"));
}

MidiController::~MidiController() {
    destroyOutputHandlers();
    // Don't close the device here. Sub-classes should close the device in their
    // destructors.
}

ControllerJSProxy* MidiController::jsProxy() {
    return new MidiControllerJSProxy(this);
}

QString MidiController::mappingExtension() {
    return MIDI_MAPPING_EXTENSION;
}

void MidiController::visit(const LegacyMidiControllerMapping* mapping) {
    m_mapping = *mapping;
    emit mappingLoaded(getMapping());
}

int MidiController::close() {
    destroyOutputHandlers();
    return 0;
}

void MidiController::visit(const LegacyHidControllerMapping* mapping) {
    Q_UNUSED(mapping);
    qWarning() << "ERROR: Attempting to load an LegacyHidControllerMapping to a MidiController!";
    // TODO(XXX): throw a hissy fit.
}

bool MidiController::matchMapping(const MappingInfo& mapping) {
    // Product info mapping not implemented for MIDI devices yet
    Q_UNUSED(mapping);
    return false;
}

bool MidiController::applyMapping() {
    // Handles the engine
    bool result = Controller::applyMapping();

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
    if (m_mapping.getOutputMappings().isEmpty()) {
        return;
    }

    QHashIterator<ConfigKey, MidiOutputMapping> outIt(m_mapping.getOutputMappings());
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
                                    QString::number(control, 16).toUpper().rightJustified(2, '0'),
                                    group,
                                    key)
                            .toUtf8();
            qWarning() << errorLog;

            int deckNum = 0;
            if (ControllerDebug::isEnabled()) {
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
        props->setText(tr(
                "One or more MixxxControls specified in the "
                "outputs section of the loaded mapping were invalid."));
        props->setInfoText(tr("Some LEDs or other feedback may not work correctly."));
        QString detailsText = tr("* Check to see that the MixxxControl "
                                 "names are spelled correctly in the mapping "
                                 "file (.xml)\n");
        detailsText += tr(
                "* Make sure the MixxxControls in question actually exist."
                " Visit the manual for a complete list: ");
        detailsText += MIXXX_MANUAL_CONTROLS_URL + QStringLiteral("\n\n");
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
    // We want to replace duplicates that exist in m_mapping but allow duplicates
    // in m_temporaryInputMappings. To do this, we first remove every key in
    // m_temporaryInputMappings from m_mapping's input mappings.
    for (auto it = m_temporaryInputMappings.constBegin();
         it != m_temporaryInputMappings.constEnd(); ++it) {
        m_mapping.removeInputMapping(it.key());
    }

    // Now, we can just use add all mappings from m_temporaryInputMappings
    // since we removed the duplicates in the original set.
    for (auto it = m_temporaryInputMappings.constBegin();
            it != m_temporaryInputMappings.constEnd();
            ++it) {
        m_mapping.addInputMapping(it.key(), it.value());
    }
    m_temporaryInputMappings.clear();
}

void MidiController::receivedShortMessage(unsigned char status,
        unsigned char control,
        unsigned char value,
        mixxx::Duration timestamp) {
    // The rest of this function is for legacy mappings
    unsigned char channel = MidiUtils::channelFromStatus(status);
    unsigned char opCode = MidiUtils::opCodeFromStatus(status);

    // Ignore MIDI beat clock messages (0xF8) until we have proper MIDI sync in
    // Mixxx. These messages are not suitable to use in JS anyway, as they are
    // sent at 24 ppqn (i.e. one message every 20.83 ms for a 120 BPM track)
    // and require real-time code. Currently, they are only spam on the
    // console, inhibit the screen saver unintentionally, could potentially
    // slow down Mixxx or interfere with the learning wizard.
    if (status == 0xF8) {
        return;
    }

    controllerDebug(MidiUtils::formatMidiMessage(getName(), status, control, value,
                                                 channel, opCode, timestamp));
    MidiKey mappingKey(status, control);

    triggerActivity();
    if (isLearning()) {
        emit messageReceived(status, control, value);

        auto it = m_temporaryInputMappings.constFind(mappingKey.key);
        if (it != m_temporaryInputMappings.constEnd()) {
            for (; it != m_temporaryInputMappings.constEnd() && it.key() == mappingKey.key; ++it) {
                processInputMapping(it.value(), status, control, value, timestamp);
            }
            return;
        }
    }

    auto it = m_mapping.getInputMappings().constFind(mappingKey.key);
    for (; it != m_mapping.getInputMappings().constEnd() && it.key() == mappingKey.key; ++it) {
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
        ControllerScriptEngineLegacy* pEngine = getScriptEngine();
        if (pEngine == nullptr) {
            return;
        }

        QJSValue function = pEngine->wrapFunctionCode(mapping.control.item, 5);
        QJSValueList args;
        args << QJSValue(channel);
        args << QJSValue(control);
        args << QJSValue(value);
        args << QJSValue(status);
        args << QJSValue(mapping.control.group);
        if (!pEngine->executeFunction(function, args)) {
            qDebug() << "MidiController: Invalid script function"
                     << mapping.control.item;
        }
        return;
    }

    // Only pass values on to valid ControlObjects.
    ControlObject* pCO = ControlObject::getControl(mapping.control);
    if (pCO == nullptr) {
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
        for (auto it = m_fourteen_bit_queued_mappings.begin();
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
        if (m_st.ignore(pCO, pCO->getParameterForMidi(newValue))) {
            return;
        }
    }
    pCO->setValueFromMidi(static_cast<MidiOpCode>(opCode), newValue);
}

double MidiController::computeValue(
        MidiOptions options, double prevmidivalue, double newmidivalue) {
    double tempval = 0.;
    double diff = 0.;

    if (options.all == 0) {
        return newmidivalue;
    }

    if (options.invert) {
        return 127. - newmidivalue;
    }

    if (options.rot64 || options.rot64_inv) {
        tempval = prevmidivalue;
        diff = newmidivalue - 64.;
        if (diff == -1 || diff == 1) {
            diff /= 16;
        } else {
            diff += (diff > 0 ? -1 : +1);
        }
        if (options.rot64) {
            tempval += diff;
        } else {
            tempval -= diff;
        }
        return (tempval < 0. ? 0. : (tempval > 127. ? 127.0 : tempval));
    }

    if (options.rot64_fast) {
        tempval = prevmidivalue;
        diff = newmidivalue - 64.;
        diff *= 1.5;
        tempval += diff;
        return (tempval < 0. ? 0. : (tempval > 127. ? 127.0 : tempval));
    }

    if (options.diff) {
        //Interpret 7-bit signed value using two's compliment.
        if (newmidivalue >= 64.) {
            newmidivalue = newmidivalue - 128.;
        }
        //Apply sensitivity to signed value. FIXME
       // if(sensitivity > 0)
        //    _newmidivalue = _newmidivalue * ((double)sensitivity / 50.);
        //Apply new value to current value.
        newmidivalue = prevmidivalue + newmidivalue;
    }

    if (options.selectknob) {
        //Interpret 7-bit signed value using two's compliment.
        if (newmidivalue >= 64.) {
            newmidivalue = newmidivalue - 128.;
        }
        //Apply sensitivity to signed value. FIXME
        //if(sensitivity > 0)
        //    _newmidivalue = _newmidivalue * ((double)sensitivity / 50.);
        //Since this is a selection knob, we do not want to inherit previous values.
    }

    if (options.button) {
        newmidivalue = newmidivalue != 0;
    }

    if (options.sw) {
        newmidivalue = 1;
    }

    if (options.spread64) {
        //qDebug() << "MIDI_OPT_SPREAD64";
        // BJW: Spread64: Distance away from centre point (aka "relative CC")
        // Uses a similar non-linear scaling formula as ControlTTRotary::getValueFromWidget()
        // but with added sensitivity adjustment. This formula is still experimental.

        newmidivalue = newmidivalue - 64.;
        //FIXME
        //double distance = _newmidivalue - 64.;
        // _newmidivalue = distance * distance * sensitivity / 50000.;
        //if (distance < 0.)
        //    _newmidivalue = -newmidivalue;

        //qDebug() << "Spread64: in " << distance << "  out " << newmidivalue;
    }

    if (options.herc_jog) {
        if (newmidivalue > 64.) {
            newmidivalue -= 128.;
        }
        newmidivalue += prevmidivalue;
        //if (_prevmidivalue != 0.0) { qDebug() << "AAAAAAAAAAAA" << prevmidivalue; }
    }

    if (options.herc_jog_fast) {
        if (newmidivalue > 64.) {
            newmidivalue -= 128.;
        }
        newmidivalue = prevmidivalue + (newmidivalue * 3);
    }

    return newmidivalue;
}

void MidiController::receive(const QByteArray& data, mixxx::Duration timestamp) {
    controllerDebug(MidiUtils::formatSysexMessage(getName(), data, timestamp));

    MidiKey mappingKey(data.at(0), 0xFF);

    triggerActivity();
    // TODO(rryan): Need to review how MIDI learn works with sysex messages. I
    // don't think this actually does anything useful.
    if (isLearning()) {
        // TODO(rryan): Fake a one value?
        emit messageReceived(mappingKey.status, mappingKey.control, 0x7F);

        auto it = m_temporaryInputMappings.constFind(mappingKey.key);
        if (it != m_temporaryInputMappings.constEnd()) {
            for (; it != m_temporaryInputMappings.constEnd() && it.key() == mappingKey.key; ++it) {
                processInputMapping(it.value(), data, timestamp);
            }
            return;
        }
    }

    auto it = m_mapping.getInputMappings().constFind(mappingKey.key);
    for (; it != m_mapping.getInputMappings().constEnd() && it.key() == mappingKey.key; ++it) {
        processInputMapping(it.value(), data, timestamp);
    }
}

void MidiController::processInputMapping(const MidiInputMapping& mapping,
                                         const QByteArray& data,
                                         mixxx::Duration timestamp) {
    // Custom script handler
    if (mapping.options.script) {
        ControllerScriptEngineLegacy* pEngine = getScriptEngine();
        if (pEngine == nullptr) {
            return;
        }
        pEngine->handleIncomingData(data);
        return;
    }
    qWarning() << "MidiController: No script function specified for"
               << MidiUtils::formatSysexMessage(getName(), data, timestamp);
}
