#ifndef KEYBOARDCONTROLLERPRESET_H
#define KEYBOARDCONTROLLERPRESET_H

#include <QHash>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/midi/midimessage.h"

class KeyboardControllerPreset : public ControllerPreset {
public:
    KeyboardControllerPreset() {
        m_KbdConfigEmpty = new ConfigObject<ConfigValueKbd>(QString());
    }
    virtual ~KeyboardControllerPreset() {}

    virtual void accept(ControllerPresetVisitor* visitor) {
        if (visitor) {
            visitor->visit(this);
        }
    }

    virtual void accept(ConstControllerPresetVisitor* visitor) const {
        if (visitor) {
            visitor->visit(this);
        }
    }

    virtual bool isMappable() const {
        return true;
    }

    // NOTE: Not functional yet. First the keyboard controller preset XML parser has to be implemented, after that
    // it will load the preset into m_KbdConfig

    // Keyboard mappings
    ConfigObject<ConfigValueKbd>* m_KbdConfig;
    ConfigObject<ConfigValueKbd>* m_KbdConfigEmpty;
};

#endif
