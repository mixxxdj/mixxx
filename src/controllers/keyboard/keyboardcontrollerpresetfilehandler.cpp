#include "util/cmdlineargs.h"
#include "util/compatibility.h"
#include "control/controlobject.h"
#include "controllers/keyboard/keyboardcontrollerpresetfilehandler.h"
#include "controllers/keyboard/layoututils.h"


KeyboardControllerPresetFileHandler::KeyboardControllerPresetFileHandler() {

}

KeyboardControllerPresetFileHandler::~KeyboardControllerPresetFileHandler() {

}

ControllerPresetPointer KeyboardControllerPresetFileHandler::load(const QDomElement root, const QString deviceName) {
    if (root.isNull()) {
        return ControllerPresetPointer();
    }

    QDomElement controller = getControllerNode(root, deviceName);
    if (controller.isNull()) {
        return ControllerPresetPointer();
    }

    KeyboardControllerPreset* preset = new KeyboardControllerPreset();

    // Get current keyboard layout
    QString layoutName = inputLocale().name();
    KeyboardLayoutPointer layout = layoutUtils::getLayout(layoutName);

    // Default to American English layout if not found
    if (layout == nullptr) {
        layoutName = "en_US";
        layout = layoutUtils::getLayout("en_US");
    }

    // Superclass handles parsing <info> tag and script files
    parsePresetInfo(root, preset);
    addScriptFilesToPreset(controller, preset);

    // Iterate through each <group> block in the XML
    QDomElement group = controller.firstChildElement("group");
    while (!group.isNull()) {
        QString groupName = group.attributeNode("name").value();

        // Iterate through each <control> element inside the current <group> block
        QDomElement control = group.firstChildElement("control");
        while (!control.isNull()) {

            // All keyseq elements (also the overloaded elements, that may
            // not be used for the user's current keyboard layout)
            QList<KbdControllerPresetKeyseq> keyseqsRaw;

            // True if there is no keyseq object which has the same
            // keyboardlayout as the user's layout. False if there is.
            bool keyseqNeedsTranslate = true;

            // Create one Keyseq struct instance for each <keyseq> element inside the
            // current <control> element and append it to keyseqsRaw
            QDomElement keyseq_element = control.firstChildElement("keyseq");
            while(!keyseq_element.isNull()) {
                keyseqsRaw.append(
                        {
                                keyseq_element.text(),                            // Key sequence
                                keyseq_element.attributeNode("lang").value(),     // Lang
                                keyseq_element.attributeNode("scancode").value()  // Scancode
                        }
                );
                keyseq_element = keyseq_element.nextSiblingElement("keyseq");
            }

            // Find action an key sequence
            QList<KbdControllerPresetKeyseq>::const_iterator keyseqsRawI = keyseqsRaw.constEnd();
            while (keyseqsRawI != keyseqsRaw.constBegin()) {
                --keyseqsRawI;

                // Check if this key sequence is a universal key and thus doesn't need a translation
                bool isUniversalKey = keyseqsRawI->scancode == "universal_key";

                // Check if this key sequence's target layout is the same as the user's language
                bool keyboardLayoutIsSame = keyseqsRawI->lang == layoutName;

                if (isUniversalKey || keyboardLayoutIsSame) {
                    keyseqNeedsTranslate = false;
                    break;
                }
            }

            QString action = control.attributeNode("action").value();
            QString keyseq = !keyseqsRaw.isEmpty() ? keyseqsRawI->keysequence : "";

            if (keyseqNeedsTranslate && !keyseqsRaw.isEmpty()) {
                QString scancode_string = keyseqsRawI->scancode;
                unsigned char scancode = (unsigned char) scancode_string.toInt();

                if (!scancode_string.isEmpty()) {
                    // Find out if the character is regular or shifted
                    QStringList modifiers = layoutUtils::getModifiersFromKeysequence(keyseq);
                    bool onlyShift = modifiers.size() == 1 && modifiers.contains("Shift");
                    Qt::KeyboardModifier modifier = onlyShift ? Qt::ShiftModifier : Qt::NoModifier;

                    // Get KbdKeyChar
                    const KbdKeyChar* keyChar = layoutUtils::getKbdKeyChar(layout, scancode, modifier);
                    QChar character = QChar(keyChar->character);

                    // If key is not dead, reconstruct key sequence with translated character.
                    // Otherwise, warn the user about the key that is not going to work.
                    if (!keyChar->is_dead) {
                        QString modifiersString = modifiers.join("+");
                        if (!modifiersString.isEmpty()) {
                            modifiersString += "+";
                        }
                        keyseq = modifiersString + character;
                    } else {
                        qWarning() << "Can't use key with scancode " << scancode
                                   << " because it's a dead key on layout '" << layoutName
                                   << "'. Please specify <keyseq lang=\"" << layoutName
                                   << "\" scancode=\"" << scancode << "\"> in the keyboard preset file (group: "
                                   << groupName << ").";
                        keyseq = "";
                    }

                }
            }

            // Load action into preset
            ConfigKey configKey = ConfigKey(groupName, action);
            preset->m_mapping.insert(keyseq, configKey);

            // Store raw data so that it can be accessed later when saving the preset
            preset->m_mapping_raw.append(
                    {configKey, keyseqsRaw}
            );

            control = control.nextSiblingElement("control");
        }

        group = group.nextSiblingElement("group");
    }

    return ControllerPresetPointer(preset);
}

bool KeyboardControllerPresetFileHandler::save(const KeyboardControllerPreset &preset, const QString deviceName,
                                               const QString fileName) const {
    QDomDocument doc = buildRootWithScripts(preset, deviceName);
    addControlsToDocument(preset, &doc);
    return writeDocument(doc, fileName);
}

void KeyboardControllerPresetFileHandler::addControlsToDocument(const KeyboardControllerPreset& preset,
                                                                QDomDocument* doc) const {
    QDomElement controller = doc->documentElement().firstChildElement("controller");

    // Group blocks will be inflated here
    QMultiHash<QString, QDomElement> groupNodes;
    QMultiHash<QString, QDomElement>::iterator groupNodesIterator;

    // Iterate over all key sequences bound to one or more actions
    const QList<KbdControllerPresetControl> mapping = preset.m_mapping_raw;
    QList<KbdControllerPresetControl>::const_iterator iterator;

    for (iterator = mapping.begin(); iterator != mapping.end(); ++iterator) {
        QString groupName = iterator->configKey.group;
        QString action = iterator->configKey.item;

        groupNodesIterator = groupNodes.find(groupName);

        // Inflate group block if it doesn't exist yet
        if (groupNodesIterator == groupNodes.end()) {
            QDomElement groupElement = doc->createElement("group");
            groupElement.setTagName("group");
            groupElement.setAttribute("name", groupName);
            groupNodesIterator = groupNodes.insert(groupName, groupElement);
        }

        Q_ASSERT(groupNodesIterator != groupNodes.end());
        QDomElement& groupNode = groupNodesIterator.value();

        // Inflate control node and append it to the group block
        QDomElement controlNode = doc->createElement("control");
        controlNode.setAttribute("action", action);

        // Create <keyseq> element for each keyseq of current PresetControl
        for (auto &keyseq : iterator->keyseqs) {
            QDomElement keyseqNode = doc->createElement("keyseq");
            keyseqNode.setAttribute("lang", keyseq.lang);
            keyseqNode.setAttribute("scancode", keyseq.scancode);
            QDomText keyseqText = doc->createTextNode(keyseq.keysequence);
            keyseqNode.appendChild(keyseqText);
            controlNode.appendChild(keyseqNode);
        }

        groupNode.appendChild(controlNode);
    }

    // Append all group blocks to the controller node
    for (groupNodesIterator = groupNodes.begin();
         groupNodesIterator != groupNodes.end(); ++groupNodesIterator) {
        controller.appendChild(groupNodesIterator.value());
    }
}