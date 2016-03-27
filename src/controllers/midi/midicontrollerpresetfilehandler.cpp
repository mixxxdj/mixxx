/**
* @file midicontrollerpresetfilehandler.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Handles loading and saving of MIDI controller presets.
*
*/

#include "controllers/midi/midicontrollerpresetfilehandler.h"
#include "controllers/midi/midiutils.h"

#define DEFAULT_OUTPUT_MAX  1.0
#define DEFAULT_OUTPUT_MIN  0.0    // Anything above 0 is "on"
#define DEFAULT_OUTPUT_ON   0x7F
#define DEFAULT_OUTPUT_OFF  0x00

ControllerPresetPointer MidiControllerPresetFileHandler::load(const QDomElement root,
                                                              const QString deviceName) {
    if (root.isNull()) {
        return ControllerPresetPointer();
    }

    QDomElement controller = getControllerNode(root, deviceName);
    if (controller.isNull()) {
        return ControllerPresetPointer();
    }

    MidiControllerPreset* preset = new MidiControllerPreset();

    // Superclass handles parsing <info> tag and script files
    parsePresetInfo(root, preset);
    addScriptFilesToPreset(controller, preset);

    QDomElement control = controller.firstChildElement("controls").firstChildElement("control");

    // Iterate through each <control> block in the XML
    while (!control.isNull()) {

        //Unserialize these objects from the XML
        QString strMidiStatus = control.firstChildElement("status").text();
        QString midiNo = control.firstChildElement("midino").text();

        bool ok = false;

        // Allow specifying hex, octal, or decimal.
        unsigned char midiStatusByte = strMidiStatus.toInt(&ok, 0);
        if (!ok) midiStatusByte = 0x00;

        // Allow specifying hex, octal, or decimal.
        unsigned char midiControl = midiNo.toInt(&ok, 0);
        if (!ok) midiControl = 0x00;

        QDomElement groupNode = control.firstChildElement("group");
        QDomElement keyNode = control.firstChildElement("key");
        QDomElement descriptionNode = control.firstChildElement("description");

        QString controlGroup = groupNode.text();
        QString controlKey = keyNode.text();
        QString controlDescription = descriptionNode.text();

        // Get options
        QDomElement optionsNode = control.firstChildElement("options").firstChildElement();

        MidiOptions options;

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
            if (strMidiOption == "hercjogfast")  options.herc_jog_fast = true;
            if (strMidiOption == "spread64") options.spread64 = true;
            if (strMidiOption == "selectknob")options.selectknob = true;
            if (strMidiOption == "soft-takeover") options.soft_takeover = true;
            if (strMidiOption == "script-binding") options.script = true;
            if (strMidiOption == "fourteen-bit-msb") options.fourteen_bit_msb = true;
            if (strMidiOption == "fourteen-bit-lsb") options.fourteen_bit_lsb = true;

            optionsNode = optionsNode.nextSiblingElement();
        }

        // Add the static mapping
        MidiInputMapping mapping;
        mapping.control = ConfigKey(controlGroup, controlKey);
        mapping.description = controlDescription;
        mapping.options = options;
        mapping.key = MidiKey(midiStatusByte, midiControl);

        // qDebug() << "New mapping:" << QString::number(mapping.key.key, 16).toUpper()
        //          << QString::number(mapping.key.status, 16).toUpper()
        //          << QString::number(mapping.key.control, 16).toUpper()
        //          << mapping.control.group << mapping.control.item;

        // Use insertMulti because we support multiple inputs mappings for the
        // same input MidiKey.
        preset->inputMappings.insertMulti(mapping.key.key, mapping);
        control = control.nextSiblingElement("control");
    }

    qDebug() << "MidiControllerPresetFileHandler: Input mapping parsing complete.";

    // Parse static output mappings

    QDomElement output = controller.firstChildElement("outputs").firstChildElement("output");

    // Iterate through each <control> block in the XML
    while (!output.isNull()) {
        // Deserialize the control components from the XML
        QDomElement groupNode = output.firstChildElement("group");
        QDomElement keyNode = output.firstChildElement("key");
        QDomElement descriptionNode = output.firstChildElement("description");

        QString controlGroup = groupNode.text();
        QString controlKey = keyNode.text();
        QString controlDescription = descriptionNode.text();

        // Unserialize output message from the XML
        MidiOutputMapping mapping;
        mapping.controlKey = ConfigKey(controlGroup, controlKey);
        mapping.description = controlDescription;

        QString midiStatus = output.firstChildElement("status").text();
        QString midiNo = output.firstChildElement("midino").text();
        QString midiOn = output.firstChildElement("on").text();
        QString midiOff = output.firstChildElement("off").text();

        bool ok = false;

        //Use QString with toInt base of 0 to auto convert hex values
        mapping.output.status = midiStatus.toInt(&ok, 0);
        if (!ok) mapping.output.status = 0x00;

        mapping.output.control = midiNo.toInt(&ok, 0);
        if (!ok) mapping.output.control = 0x00;

        mapping.output.on = midiOn.toInt(&ok, 0);
        if (!ok) mapping.output.on = DEFAULT_OUTPUT_ON;

        mapping.output.off = midiOff.toInt(&ok, 0);
        if (!ok) mapping.output.off = DEFAULT_OUTPUT_OFF;

        QDomElement minNode = output.firstChildElement("minimum");
        QDomElement maxNode = output.firstChildElement("maximum");

        ok = false;
        if (!minNode.isNull()) {
            mapping.output.min = minNode.text().toDouble(&ok);
        } else {
            ok = false;
        }

        if (!ok) //If not a double, or node wasn't defined
            mapping.output.min = DEFAULT_OUTPUT_MIN;

        if (!maxNode.isNull()) {
            mapping.output.max = maxNode.text().toDouble(&ok);
        } else {
            ok = false;
        }

        if (!ok) //If not a double, or node wasn't defined
            mapping.output.max = DEFAULT_OUTPUT_MAX;

        // END unserialize output

        // Add the static output mapping.
        // qDebug() << "New output mapping:"
        //          << QString::number(mapping.output.status, 16).toUpper()
        //          << QString::number(mapping.output.control, 16).toUpper()
        //          << QString::number(mapping.output.on, 16).toUpper()
        //          << QString::number(mapping.output.off, 16).toUpper()
        //          << controlGroup << controlKey;

        // Use insertMulti because we support multiple outputs from the same
        // control.
        preset->outputMappings.insertMulti(mapping.controlKey, mapping);

        output = output.nextSiblingElement("output");
    }

    qDebug() << "MidiPresetFileHandler: Output mapping parsing complete.";

    return ControllerPresetPointer(preset);
}

bool MidiControllerPresetFileHandler::save(const MidiControllerPreset& preset,
                                           const QString deviceName,
                                           const QString fileName) const {
    qDebug() << "Saving preset for" << deviceName << "to" << fileName;
    QDomDocument doc = buildRootWithScripts(preset, deviceName);
    addControlsToDocument(preset, &doc);
    return writeDocument(doc, fileName);
}

void MidiControllerPresetFileHandler::addControlsToDocument(const MidiControllerPreset& preset,
                                                            QDomDocument* doc) const {
    QDomElement controller = doc->documentElement().firstChildElement("controller");
    QDomElement controls = doc->createElement("controls");

    // Iterate over all of the command/control pairs in the input mapping
    QHashIterator<uint16_t, MidiInputMapping> it(preset.inputMappings);
    while (it.hasNext()) {
        it.next();

        const MidiInputMapping& mapping = it.value();
        QDomElement controlNode = inputMappingToXML(doc, mapping);

        // Add the control node we just created to the XML document in the
        // proper spot.
        controls.appendChild(controlNode);
    }
    controller.appendChild(controls);

    QDomElement outputs = doc->createElement("outputs");

    //Iterate over all of the command/control pairs in the OUTPUT mapping
    QHashIterator<ConfigKey, MidiOutputMapping> outIt(preset.outputMappings);
    while (outIt.hasNext()) {
        outIt.next();

        const MidiOutputMapping& mapping = outIt.value();
        QDomElement outputNode = outputMappingToXML(doc, mapping);

        // Add the control node we just created to the XML document in the
        // proper spot.
        outputs.appendChild(outputNode);
    }
    controller.appendChild(outputs);
}

QDomElement MidiControllerPresetFileHandler::makeTextElement(QDomDocument* doc,
                                                             const QString& elementName,
                                                             const QString& text) const {
    QDomElement tagNode = doc->createElement(elementName);
    QDomText textNode = doc->createTextNode(text);
    tagNode.appendChild(textNode);
    return tagNode;
}

QDomElement MidiControllerPresetFileHandler::inputMappingToXML(
        QDomDocument* doc, const MidiInputMapping& mapping) const {
    QDomElement controlNode = doc->createElement("control");

    controlNode.appendChild(makeTextElement(doc, "group", mapping.control.group));
    controlNode.appendChild(makeTextElement(doc, "key", mapping.control.item));
    if (!mapping.description.isEmpty()) {
        controlNode.appendChild(
            makeTextElement(doc, "description", mapping.description));
    }

    // MIDI status byte
    controlNode.appendChild(makeTextElement(
        doc, "status", MidiUtils::formatByteAsHex(mapping.key.status)));

    if (mapping.key.control != 0xFF) {
        //MIDI control number
        controlNode.appendChild(makeTextElement(
            doc, "midino", MidiUtils::formatByteAsHex(mapping.key.control)));
    }

    //Midi options
    QDomElement optionsNode = doc->createElement("options");

    // "normal" is no options
    if (mapping.options.all == 0) {
        QDomElement singleOption = doc->createElement("normal");
            optionsNode.appendChild(singleOption);
    } else {
        if (mapping.options.invert) {
            QDomElement singleOption = doc->createElement("invert");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.rot64) {
            QDomElement singleOption = doc->createElement("rot64");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.rot64_inv) {
            QDomElement singleOption = doc->createElement("rot64inv");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.rot64_fast) {
            QDomElement singleOption = doc->createElement("rot64fast");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.diff) {
            QDomElement singleOption = doc->createElement("diff");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.button) {
            QDomElement singleOption = doc->createElement("button");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.sw) {
            QDomElement singleOption = doc->createElement("switch");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.herc_jog) {
            QDomElement singleOption = doc->createElement("hercjog");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.herc_jog_fast) {
            QDomElement singleOption = doc->createElement("hercjogfast");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.spread64) {
            QDomElement singleOption = doc->createElement("spread64");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.selectknob) {
            QDomElement singleOption = doc->createElement("selectknob");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.soft_takeover) {
            QDomElement singleOption = doc->createElement("soft-takeover");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.script) {
            QDomElement singleOption = doc->createElement("script-binding");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.fourteen_bit_msb) {
            QDomElement singleOption = doc->createElement("fourteen-bit-msb");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.fourteen_bit_lsb) {
            QDomElement singleOption = doc->createElement("fourteen-bit-lsb");
            optionsNode.appendChild(singleOption);
        }
    }
    controlNode.appendChild(optionsNode);

    return controlNode;
}

QDomElement MidiControllerPresetFileHandler::outputMappingToXML(
        QDomDocument* doc, const MidiOutputMapping& mapping) const {
    QDomElement outputNode = doc->createElement("output");

    outputNode.appendChild(makeTextElement(doc, "group", mapping.controlKey.group));
    outputNode.appendChild(makeTextElement(doc, "key", mapping.controlKey.item));
    if (!mapping.description.isEmpty()) {
        outputNode.appendChild(
            makeTextElement(doc, "description", mapping.description));
    }

    // MIDI status byte
    outputNode.appendChild(makeTextElement(
        doc, "status", MidiUtils::formatByteAsHex(mapping.output.status)));

    if (mapping.output.control != 0xFF) {
        //MIDI control number
        outputNode.appendChild(makeTextElement(
            doc, "midino", MidiUtils::formatByteAsHex(mapping.output.control)));
    }

    // Third MIDI byte for turning on the LED
    if (mapping.output.on != DEFAULT_OUTPUT_ON) {
        outputNode.appendChild(makeTextElement(
            doc, "on", MidiUtils::formatByteAsHex(mapping.output.on)));
    }

    // Third MIDI byte for turning off the LED
    if (mapping.output.off != DEFAULT_OUTPUT_OFF) {
        outputNode.appendChild(makeTextElement(
            doc, "off", MidiUtils::formatByteAsHex(mapping.output.off)));
    }

    // Upper value, above which the 'off' value is sent
    //  (We don't bother writing it if it's the default value.)
    if (mapping.output.max != DEFAULT_OUTPUT_MAX) {
        outputNode.appendChild(makeTextElement(
            doc, "maximum", QString::number(mapping.output.max)));
    }

    // Lower value, below which the 'off' value is sent
    //  (We don't bother writing it if it's the default value.)
    if (mapping.output.min != DEFAULT_OUTPUT_MIN) {
        outputNode.appendChild(makeTextElement(
            doc, "minimum", QString::number(mapping.output.min)));
    }

    return outputNode;
}
