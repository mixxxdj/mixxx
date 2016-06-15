#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H

#include "controllers/controller.h"
#include "controllers/keyboard/keyboardcontrollerpreset.h"
#include "keyboardeventfilter.h"

class KeyboardController : public Controller {
Q_OBJECT

public:
    KeyboardController(KeyboardEventFilter* pKbdEventFilter);
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

public slots:
    // Sets a control, only if the keyboard is enabled
    void onKeySeqPressed(ConfigKey configKey);

private slots:
    virtual int open();
    virtual int close();

private:
    virtual void send(QByteArray data) {
        // Keyboard is an input-only controller, so this
        // method doesn't need any implementation
        Q_UNUSED(data);
    }

    virtual inline bool isPolling() const {
        return false;
    }

    virtual ControllerPreset* preset();

    KeyboardControllerPreset m_preset;

    KeyboardEventFilter* m_pKbdEventFilter;
};


#endif // KEYBOARDCONTROLLER_H
