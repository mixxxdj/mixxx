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

    QDomElement group = controller.firstChildElement("group");

    // Iterate through each <group> block in the XML
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