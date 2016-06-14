#include "keyboardcontrollerpresetfilehandler.h"


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

    // Superclass handles parsing <info> tag and script files
    parsePresetInfo(root, preset);
    addScriptFilesToPreset(controller, preset);

    // Iterate through each <group> block in the XML
    QDomElement group = controller.firstChildElement("group");
    while (!group.isNull()) {
        QString groupName = group.attributeNode("name").value();
        QDomElement control = group.firstChildElement("control");

        // Iterate through each <control> node inside the current <group> block
        while (!control.isNull()) {
            QString action = control.attributeNode("action").value();
            QString keyseq = control.attributeNode("keyseq").value();

            ConfigValueKbd configValueKbd = ConfigValueKbd(keyseq);
            ConfigKey configKey = ConfigKey(groupName, action);

            // Load action into preset
            preset->m_keySequenceToControlHash.insert(configValueKbd, configKey);

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
    const QMultiHash<ConfigValueKbd, ConfigKey>& mapping = preset.m_keySequenceToControlHash;
    QMultiHash<ConfigValueKbd, ConfigKey>::const_iterator iterator;

    for (iterator = mapping.begin(); iterator != mapping.end(); ++iterator) {
        const ConfigValueKbd& configValueKbd = iterator.key();
        QString groupName = iterator.value().group;
        QString action = iterator.value().item;

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
        controlNode.setAttribute("keyseq", configValueKbd.value);
        groupNode.appendChild(controlNode);
    }

    // Append all group blocks to the controller node
    for (groupNodesIterator = groupNodes.begin();
         groupNodesIterator != groupNodes.end(); ++groupNodesIterator) {
        controller.appendChild(groupNodesIterator.value());
    }
}