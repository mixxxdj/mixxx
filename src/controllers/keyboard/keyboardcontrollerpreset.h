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

    QList<QKeySequence> getKeySequences(ConfigKey configKey) {
        QList<QKeySequence> keySeqs;
        QMultiHash<ConfigValueKbd, ConfigKey>::iterator it;
        for (it = m_keySequenceToControlHash.begin(); it != m_keySequenceToControlHash.end(); ++it) {
            const ConfigKey& currentConfigKey = it.value();
            if (currentConfigKey == configKey) {
                ConfigValueKbd configValueKbd = it.key();
                keySeqs.append(configValueKbd.m_qKey);
            }
        }
        return keySeqs;
    }

    QMultiHash<ConfigValueKbd, ConfigKey> getMappingByGroup(QString targetGroup) {
        QMultiHash<ConfigValueKbd, ConfigKey> filteredKeySequenceHash;
        QMultiHash<ConfigValueKbd, ConfigKey>::iterator it;
        for (it = m_keySequenceToControlHash.begin(); it != m_keySequenceToControlHash.end(); ++it) {
            QString currentGroup = it.value().group;
            if (currentGroup == targetGroup) {
                filteredKeySequenceHash.insert(it.key(), it.value());
            }
        }
        return filteredKeySequenceHash;
    };

    // Multi-hash of config keys, bound to a specific key sequence
    QMultiHash<ConfigValueKbd, ConfigKey> m_keySequenceToControlHash;
};

#endif
