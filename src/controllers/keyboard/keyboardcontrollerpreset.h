#ifndef KEYBOARDCONTROLLERPRESET_H
#define KEYBOARDCONTROLLERPRESET_H

#include <QHash>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/midi/midimessage.h"
#include "controllers/keyboard/layoututils.h"

class KeyboardControllerPreset : public ControllerPreset {
  public:
    KeyboardControllerPreset() {}
    virtual ~KeyboardControllerPreset() {}

    void accept(ControllerPresetVisitor* visitor) override {
        if (visitor) {
            visitor->visitKeyboard(this);
        }
    }

    void accept(ConstControllerPresetVisitor* visitor) const override {
        if (visitor) {
            visitor->visitKeyboard(this);
        }
    }

    bool isMappable() const override { return true; }

    void translate(QString layout);

    // Get all QKeySequences bound to a given ConfigKey, separated by a given separator
    QString getKeySequencesToString(ConfigKey configKey, QString separator) const;

    // Get all QKeySequences bound to a given ConfigKey
    QStringList getKeySequences(ConfigKey configKey) const;

    // Get mapping info filtered by a given group name
    QMultiHash<QString, ConfigKey> getMappingByGroup(const QString& targetGroup) const;

    // Multi-hash of config keys, bound to a specific key sequence
    QMultiHash<QString, ConfigKey> m_mapping;

    // List of Control structs, one for each parsed <control> element in the preset file
    QList<KbdControllerPresetControl> m_mappingRaw;
};

typedef QSharedPointer<KeyboardControllerPreset> KeyboardControllerPresetPointer;

#endif
