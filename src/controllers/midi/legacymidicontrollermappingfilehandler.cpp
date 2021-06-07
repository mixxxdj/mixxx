#include "controllers/midi/legacymidicontrollermappingfilehandler.h"

#include "controllers/midi/midiutils.h"

#define DEFAULT_OUTPUT_MAX 1.0
#define DEFAULT_OUTPUT_MIN 0.0 // Anything above 0 is "on"
#define DEFAULT_OUTPUT_ON 0x7F
#define DEFAULT_OUTPUT_OFF 0x00

std::shared_ptr<LegacyControllerMapping>
LegacyMidiControllerMappingFileHandler::load(const QDomElement& root,
        const QString& filePath,
        const QDir& systemMappingsPath) {
    if (root.isNull()) {
        return nullptr;
    }

    QDomElement controller = getControllerNode(root);
    if (controller.isNull()) {
        return nullptr;
    }

    auto pMapping = std::make_shared<LegacyMidiControllerMapping>();
    pMapping->setFilePath(filePath);

    // Superclass handles parsing <info> tag and script files
    parseMappingInfo(root, pMapping);
    addScriptFilesToMapping(controller, pMapping, systemMappingsPath);

    QDomElement control = controller.firstChildElement("controls").firstChildElement("control");

    // Iterate through each <control> block in the XML
    while (!control.isNull()) {
        //Unserialize these objects from the XML
        QString strMidiStatus = control.firstChildElement("status").text();
        QString midiNo = control.firstChildElement("midino").text();

        bool ok = false;

        // Allow specifying hex, octal, or decimal.
        unsigned char midiStatusByte = strMidiStatus.toInt(&ok, 0);
        if (!ok) {
            midiStatusByte = 0x00;
        }

        // Allow specifying hex, octal, or decimal.
        unsigned char midiControl = midiNo.toInt(&ok, 0);
        if (!ok) {
            midiControl = 0x00;
        }

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
            if (strMidiOption == QLatin1String("invert")) {
                options.setFlag(MidiOption::Invert);
            } else if (strMidiOption == QLatin1String("rot64")) {
                options.setFlag(MidiOption::Rot64);
            } else if (strMidiOption == QLatin1String("rot64inv")) {
                options.setFlag(MidiOption::Rot64Invert);
            } else if (strMidiOption == QLatin1String("rot64fast")) {
                options.setFlag(MidiOption::Rot64Fast);
            } else if (strMidiOption == QLatin1String("diff")) {
                options.setFlag(MidiOption::Diff);
            } else if (strMidiOption == QLatin1String("button")) {
                options.setFlag(MidiOption::Button);
            } else if (strMidiOption == QLatin1String("switch")) {
                options.setFlag(MidiOption::Switch);
            } else if (strMidiOption == QLatin1String("hercjog")) {
                options.setFlag(MidiOption::HercJog);
            } else if (strMidiOption == QLatin1String("hercjogfast")) {
                options.setFlag(MidiOption::HercJogFast);
            } else if (strMidiOption == QLatin1String("spread64")) {
                options.setFlag(MidiOption::Spread64);
            } else if (strMidiOption == QLatin1String("selectknob")) {
                options.setFlag(MidiOption::SelectKnob);
            } else if (strMidiOption == QLatin1String("soft-takeover")) {
                options.setFlag(MidiOption::SoftTakeover);
            } else if (strMidiOption == QLatin1String("script-binding")) {
                options.setFlag(MidiOption::Script);
            } else if (strMidiOption == QLatin1String("fourteen-bit-msb")) {
                options.setFlag(MidiOption::FourteenBitMSB);
            } else if (strMidiOption == QLatin1String("fourteen-bit-lsb")) {
                options.setFlag(MidiOption::FourteenBitLSB);
            }

            optionsNode = optionsNode.nextSiblingElement();
        }

        MidiInputMapping inputMapping;
        inputMapping.control = ConfigKey(controlGroup, controlKey);
        inputMapping.description = controlDescription;
        inputMapping.options = options;
        inputMapping.key = MidiKey(midiStatusByte, midiControl);

        // qDebug() << "New inputMapping:" << QString::number(inputMapping.key.key, 16).toUpper()
        //          << QString::number(inputMapping.key.status, 16).toUpper()
        //          << QString::number(inputMapping.key.control, 16).toUpper()
        //          << inputMapping.control.group << inputMapping.control.item;

        // Use insertMulti because we support multiple inputs mappings for the
        // same input MidiKey.
        pMapping->addInputMapping(inputMapping.key.key, inputMapping);
        control = control.nextSiblingElement("control");
    }

    qDebug() << "LegacyMidiControllerMappingFileHandler: Input mapping parsing complete.";

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
        MidiOutputMapping outputMapping;
        outputMapping.controlKey = ConfigKey(controlGroup, controlKey);
        outputMapping.description = controlDescription;

        QString midiStatus = output.firstChildElement("status").text();
        QString midiNo = output.firstChildElement("midino").text();
        QString midiOn = output.firstChildElement("on").text();
        QString midiOff = output.firstChildElement("off").text();

        bool ok = false;

        //Use QString with toInt base of 0 to auto convert hex values
        outputMapping.output.status = midiStatus.toInt(&ok, 0);
        if (!ok) {
            outputMapping.output.status = 0x00;
        }

        outputMapping.output.control = midiNo.toInt(&ok, 0);
        if (!ok) {
            outputMapping.output.control = 0x00;
        }

        outputMapping.output.on = midiOn.toInt(&ok, 0);
        if (!ok) {
            outputMapping.output.on = DEFAULT_OUTPUT_ON;
        }

        outputMapping.output.off = midiOff.toInt(&ok, 0);
        if (!ok) {
            outputMapping.output.off = DEFAULT_OUTPUT_OFF;
        }

        QDomElement minNode = output.firstChildElement("minimum");
        QDomElement maxNode = output.firstChildElement("maximum");

        ok = false;
        if (!minNode.isNull()) {
            outputMapping.output.min = minNode.text().toDouble(&ok);
        } else {
            ok = false;
        }

        if (!ok) {
            // If not a double, or node wasn't defined
            outputMapping.output.min = DEFAULT_OUTPUT_MIN;
        }

        if (!maxNode.isNull()) {
            outputMapping.output.max = maxNode.text().toDouble(&ok);
        } else {
            ok = false;
        }

        if (!ok) { //If not a double, or node wasn't defined
            outputMapping.output.max = DEFAULT_OUTPUT_MAX;
        }

        // END unserialize output

        // Add the static output mapping.
        // qDebug() << "New output mapping:"
        //          << QString::number(outputMapping.output.status, 16).toUpper()
        //          << QString::number(outputMapping.output.control, 16).toUpper()
        //          << QString::number(outputMapping.output.on, 16).toUpper()
        //          << QString::number(outputMapping.output.off, 16).toUpper()
        //          << controlGroup << controlKey;

        // Use insertMulti because we support multiple outputs from the same
        // control.
        pMapping->addOutputMapping(outputMapping.controlKey, outputMapping);

        output = output.nextSiblingElement("output");
    }

    qDebug() << "MidiMappingFileHandler: Output mapping parsing complete.";

    return pMapping;
}

bool LegacyMidiControllerMappingFileHandler::save(const LegacyMidiControllerMapping& mapping,
        const QString& fileName) const {
    qDebug() << "Saving mapping" << mapping.name() << "to" << fileName;
    QDomDocument doc = buildRootWithScripts(mapping);
    addControlsToDocument(mapping, &doc);
    return writeDocument(doc, fileName);
}

void LegacyMidiControllerMappingFileHandler::addControlsToDocument(
        const LegacyMidiControllerMapping& mapping, QDomDocument* doc) const {
    QDomElement controller = doc->documentElement().firstChildElement("controller");

    // The QHash doesn't guarantee iteration order, so first we sort the keys
    // so the xml output will be consistent.
    QDomElement controls = doc->createElement("controls");
    // We will iterate over all of the values that have the same keys, so we need
    // to remove duplicate keys or else we'll duplicate those values.
    auto sortedInputKeys = mapping.getInputMappings().uniqueKeys();
    std::sort(sortedInputKeys.begin(), sortedInputKeys.end());
    for (const auto& key : sortedInputKeys) {
        for (auto it = mapping.getInputMappings().constFind(key);
                it != mapping.getInputMappings().constEnd() && it.key() == key;
                ++it) {
            QDomElement controlNode = inputMappingToXML(doc, it.value());
            controls.appendChild(controlNode);
        }
    }
    controller.appendChild(controls);

    // Repeat the process for the output mappings.
    QDomElement outputs = doc->createElement("outputs");
    auto sortedOutputKeys = mapping.getOutputMappings().uniqueKeys();
    std::sort(sortedOutputKeys.begin(), sortedOutputKeys.end());
    for (const auto& key : sortedOutputKeys) {
        for (auto it = mapping.getOutputMappings().constFind(key);
                it != mapping.getOutputMappings().constEnd() && it.key() == key;
                ++it) {
            QDomElement outputNode = outputMappingToXML(doc, it.value());
            outputs.appendChild(outputNode);
        }
    }
    controller.appendChild(outputs);
}

QDomElement LegacyMidiControllerMappingFileHandler::makeTextElement(QDomDocument* doc,
        const QString& elementName,
        const QString& text) const {
    QDomElement tagNode = doc->createElement(elementName);
    QDomText textNode = doc->createTextNode(text);
    tagNode.appendChild(textNode);
    return tagNode;
}

QDomElement LegacyMidiControllerMappingFileHandler::inputMappingToXML(
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
    if (!mapping.options) {
        QDomElement singleOption = doc->createElement("normal");
        optionsNode.appendChild(singleOption);
    } else {
        if (mapping.options.testFlag(MidiOption::Invert)) {
            QDomElement singleOption = doc->createElement("invert");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::Rot64)) {
            QDomElement singleOption = doc->createElement("rot64");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::Rot64Invert)) {
            QDomElement singleOption = doc->createElement("rot64inv");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::Rot64Fast)) {
            QDomElement singleOption = doc->createElement("rot64fast");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::Diff)) {
            QDomElement singleOption = doc->createElement("diff");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::Button)) {
            QDomElement singleOption = doc->createElement("button");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::Switch)) {
            QDomElement singleOption = doc->createElement("switch");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::HercJog)) {
            QDomElement singleOption = doc->createElement("hercjog");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::HercJogFast)) {
            QDomElement singleOption = doc->createElement("hercjogfast");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::Spread64)) {
            QDomElement singleOption = doc->createElement("spread64");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::SelectKnob)) {
            QDomElement singleOption = doc->createElement("selectknob");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::SoftTakeover)) {
            QDomElement singleOption = doc->createElement("soft-takeover");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::Script)) {
            QDomElement singleOption = doc->createElement("script-binding");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::FourteenBitMSB)) {
            QDomElement singleOption = doc->createElement("fourteen-bit-msb");
            optionsNode.appendChild(singleOption);
        }
        if (mapping.options.testFlag(MidiOption::FourteenBitLSB)) {
            QDomElement singleOption = doc->createElement("fourteen-bit-lsb");
            optionsNode.appendChild(singleOption);
        }
    }
    controlNode.appendChild(optionsNode);

    return controlNode;
}

QDomElement LegacyMidiControllerMappingFileHandler::outputMappingToXML(
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
