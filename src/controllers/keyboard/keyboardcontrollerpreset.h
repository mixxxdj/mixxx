#ifndef KEYBOARDCONTROLLERPRESET_H
#define KEYBOARDCONTROLLERPRESET_H

#include <QHash>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/midi/midimessage.h"

class KeyboardControllerPreset : public ControllerPreset {
public:
    KeyboardControllerPreset() {}
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

    // Multi-hash of config keys, bound to a specific key sequence
    QMultiHash<ConfigValueKbd, ConfigKey> m_keySequenceToControlHash;
};

#endif
