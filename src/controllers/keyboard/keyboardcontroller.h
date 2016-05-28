#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H


#include "controllers/controller.h"
#include "controllers/keyboard/keyboardcontrollerpreset.h"

// This class can't be instantiated yet, because the virtual methods: "void send(QByteArray)", "bool isPolling()" and "ControllerPreset *preset()" aren't yet implemented
// TODO(Tomasito) What is the best way to implement this methods? Subclassing KeyboardController, just as MidiController does? Or just implement them here? (I think the latter)

class KeyboardController : public Controller {
Q_OBJECT

public:
    KeyboardController();
    virtual ~KeyboardController();

    virtual bool eventFilter(QObject*, QEvent* e);

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

signals:
    // TODO(Tomasito) onKeyPressed signal? check how it's done with the current keyboard implementation

private slots:
    virtual int open();
    virtual int close();

private:
    KeyboardControllerPreset m_preset;
};


#endif // KEYBOARDCONTROLLER_H
