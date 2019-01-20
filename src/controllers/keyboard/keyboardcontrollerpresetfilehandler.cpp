#include "util/cmdlineargs.h"
#include "util/compatibility.h"
#include "control/controlobject.h"
#include "controllers/keyboard/keyboardcontrollerpresetfilehandler.h"
#include "controllers/keyboard/layoututils.h"


KeyboardControllerPresetFileHandler::KeyboardControllerPresetFileHandler() {
}

KeyboardControllerPresetFileHandler::~KeyboardControllerPresetFileHandler() {
}

ControllerPresetPointer KeyboardControllerPresetFileHandler::load(
        const QDomElement root, const QString deviceName) {
    if (root.isNull()) {
        return ControllerPresetPointer();
    }

    QDomElement controller = getControllerNode(root, deviceName);
    if (controller.isNull()) {
        return ControllerPresetPointer();
    }

    KeyboardControllerPreset* preset = new KeyboardControllerPreset();

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

            // Create one Keyseq struct instance for each <keyseq> element inside the
            // current <control> element and append it to keyseqsRaw
            QDomElement keyseq_element = control.firstChildElement("keyseq");
            while(!keyseq_element.isNull()) {
                keyseqsRaw.append(
                        {
                                keyseq_element.text(),                                // Key sequence
                                keyseq_element.attributeNode("lang").value(),         // Lang
                                keyseq_element.attributeNode("byPositionOf").value()  // By position of lang

                        }
                );
                keyseq_element = keyseq_element.nextSiblingElement("keyseq");
            }

            // Store raw data so that it can be accessed later when saving the preset
            QString action = control.attributeNode("action").value();
            ConfigKey configKey = ConfigKey(groupName, action);
            preset->m_mappingRaw.append(
                    {configKey, keyseqsRaw}
            );

            control = control.nextSiblingElement("control");
        }

        group = group.nextSiblingElement("group");
    }

    // Translate preset to current keyboard layout
    preset->translate(inputLocale().name());

    return ControllerPresetPointer(preset);
}

bool KeyboardControllerPresetFileHandler::save(const KeyboardControllerPreset& preset, const QString& deviceName,
                                               const QString& fileName) const {
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
    const QList<KbdControllerPresetControl> mapping = preset.m_mappingRaw;
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
        for (const auto& keyseq : iterator->keyseqs) {
            QDomElement keyseqNode = doc->createElement("keyseq");
            if (!keyseq.lang.isEmpty())
                keyseqNode.setAttribute("lang", keyseq.lang);
            if (!keyseq.byPositionOf.isEmpty())
                keyseqNode.setAttribute("byPositionOf", keyseq.byPositionOf);
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
