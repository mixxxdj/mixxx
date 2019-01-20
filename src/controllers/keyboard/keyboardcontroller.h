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
    explicit KeyboardController(KeyboardEventFilter* pKbdEventFilter);
    ~KeyboardController() override;

    QString presetExtension() override;

    ControllerPresetPointer getPreset() const override {
        KeyboardControllerPreset* pClone = new KeyboardControllerPreset();
        *pClone = m_preset;
        return ControllerPresetPointer(pClone);
    }

    KeyboardControllerPresetPointer getKeyboardPreset() const {
        KeyboardControllerPreset* pClone = new KeyboardControllerPreset();
        *pClone = m_preset;
        return KeyboardControllerPresetPointer(pClone);
    }

    bool savePreset(const QString fileName) const override;
    bool matchPreset(const PresetInfo& preset) override;
    void visitKeyboard(const KeyboardControllerPreset* preset) override;
    void visitMidi(const MidiControllerPreset* preset) override;
    void visitHid(const HidControllerPreset* preset) override;
    void accept(ControllerVisitor* visitor) override {
        if (visitor) {
            visitor->visit(this);
        }
    }
    inline bool isMappable() const override { return m_preset.isMappable(); }

  public slots:
    // Sets a control, only if the keyboard is enabled
    void onKeySeqPressed(ConfigKey configKey);
    void reloadPreset(QString layout);

  signals:
    void keyboardControllerPresetLoaded(KeyboardControllerPresetPointer presetPointer);
    void enabled(bool);

  private:
    void send(QByteArray data) override {
        // Keyboard is an input-only controller, so this
        // method doesn't need any implementation
        Q_UNUSED(data);
    }

    inline bool isPolling() const override { return false; }
    inline ControllerPreset* preset() override { return &m_preset; }

    // Keyboard controller preset, holding mapping info
    KeyboardControllerPreset m_preset;

    // Pointer to KeyboardEventFilter, owned by MixxxMainWindow
    KeyboardEventFilter* m_pKbdEventFilter;

  private slots:
    int open() override;
    int close() override;
};

typedef QSharedPointer<KeyboardController> KeyboardControllerPointer;

#endif // KEYBOARDCONTROLLER_H
