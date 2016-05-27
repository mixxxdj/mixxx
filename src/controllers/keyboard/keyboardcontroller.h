#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H


#include "controllers/controller.h"
#include "controllers/keyboard/keyboardcontrollerpreset.h"

class KeyboardController : public Controller {
Q_OBJECT

public:
    KeyboardController();
    virtual ~KeyboardController();

    virtual QString presetExtension();

    virtual ControllerPresetPointer getPreset() const {
        KeyboardControllerPreset* pClone = new KeyboardControllerPreset();
        *pClone = m_preset;
        return ControllerPresetPointer(pClone);
    }

    virtual bool savePreset(const QString fileName) const;

    virtual void visit(const KeyboardControllerPreset* preset);
    virtual void visit(const MidiControllerPreset* preset);
    virtual void visit(const HidControllerPreset* preset);

    virtual void accept(ControllerVisitor* visitor) {
        if (visitor) {
            visitor->visit(this);
        }
    }

    virtual bool isMappable() const {
        return m_preset.isMappable();
    }

    virtual bool matchPreset(const PresetInfo& preset);

private:
    KeyboardControllerPreset m_preset;
};


#endif // KEYBOARDCONTROLLER_H
