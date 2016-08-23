#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H

#include "controllers/controller.h"
#include "controllers/keyboard/keyboardcontrollerpreset.h"
#include "keyboardeventfilter.h"

// KeyboardController is the controller for the keyboard, holding a reference to the KeyboardEventFilter, which is
// sent a signal when a new preset --new keyboard mapping-- is loaded.
class KeyboardController : public Controller {
    Q_OBJECT
  public:
    KeyboardController(KeyboardEventFilter* pKbdEventFilter);
    virtual ~KeyboardController();

    virtual QString presetExtension() override;

    virtual ControllerPresetPointer getPreset() const override {
        KeyboardControllerPreset* pClone = new KeyboardControllerPreset();
        *pClone = m_preset;
        return ControllerPresetPointer(pClone);
    }

    KeyboardControllerPresetPointer getKeyboardPreset() const {
        KeyboardControllerPreset* pClone = new KeyboardControllerPreset();
        *pClone = m_preset;
        return KeyboardControllerPresetPointer(pClone);
    }

    virtual bool savePreset(const QString fileName) const override;
    virtual bool matchPreset(const PresetInfo& preset) override;
    virtual void visit(const KeyboardControllerPreset* preset) override;
    virtual void visit(const MidiControllerPreset* preset) override;
    virtual void visit(const HidControllerPreset* preset) override;
    virtual void accept(ControllerVisitor* visitor) {
        if (visitor) {
            visitor->visit(this);
        }
    }
    inline virtual bool isMappable() const override { return m_preset.isMappable(); }

  public slots:
    // Sets a control, only if the keyboard is enabled
    void onKeySeqPressed(ConfigKey configKey);
    void reloadPreset(QString layout);

  signals:
    void keyboardControllerPresetLoaded(KeyboardControllerPresetPointer presetPointer);
    void enabled(bool);

  private:
    virtual void send(QByteArray data) override {
        // Keyboard is an input-only controller, so this
        // method doesn't need any implementation
        Q_UNUSED(data);
    }

    inline virtual bool isPolling() const override { return false; }
    inline virtual ControllerPreset* preset() override { return &m_preset; }

    // Keyboard controller preset, holding mapping info
    KeyboardControllerPreset m_preset;

    // Pointer to KeyboardEventFilter, owned by MixxxMainWindow
    KeyboardEventFilter* m_pKbdEventFilter;

  private slots:
    virtual int open() override;
    virtual int close() override;
};

typedef QSharedPointer<KeyboardController> KeyboardControllerPointer;

#endif // KEYBOARDCONTROLLER_H
