#pragma once

#include <QMultiHash>
#include <QObject>

#include "control/controlobject.h"
#include "preferences/configobject.h"

class ControlObject;
class QEvent;
class QKeyEvent;

// This class provides handling of keyboard events.
class KeyboardEventFilter : public QObject {
    Q_OBJECT
  public:
    KeyboardEventFilter(ConfigObject<ConfigValueKbd> *pKbdConfigObject,
                        QObject *parent = nullptr, const char* name = nullptr);
    virtual ~KeyboardEventFilter();

    bool eventFilter(QObject* obj, QEvent* e);

    // Set the keyboard config object. KeyboardEventFilter does NOT take
    // ownership of pKbdConfigObject.
    void setKeyboardConfig(ConfigObject<ConfigValueKbd> *pKbdConfigObject);
    ConfigObject<ConfigValueKbd>* getKeyboardConfig();

    // Returns a valid QString with modifier keys from a QKeyEvent
    static QKeySequence getKeySeq(QKeyEvent* e);

    // Key learning mode
    void setLearningMode(bool enable);
    bool isLearningMode() const { return m_learningMode; }

    void setStrictMode(bool enable) { m_strictMode = enable; }
    bool isStrictMode() const { return m_strictMode; }

  signals:
#ifndef __APPLE__
    void altPressedWithoutKeys();
#endif
    // Emitted when a key is captured in learning mode
    void keyCaptured(QKeySequence keySequence);
    // Emitted for every key press (for visual feedback/monitoring)
    void keyPressed(QKeySequence keySequence);

  private:
    struct KeyDownInformation {
        KeyDownInformation(int keyId, int modifiers, ControlObject* pControl)
                : keyId(keyId),
                  modifiers(modifiers),
                  pControl(pControl) {
        }

        int keyId;
        int modifiers;
        ControlObject* pControl;
    };

#ifndef __APPLE__
    bool m_altPressedWithoutKey;
#endif

    // Run through list of active keys to see if the pressed key is already active
    // and is not a control that repeats when held.
    bool shouldSkipHeldKey(int keyId) {
        return std::any_of(
                m_qActiveKeyList.cbegin(),
                m_qActiveKeyList.cend(),
                [&](const KeyDownInformation& keyDownInfo) {
                    return keyDownInfo.keyId == keyId && !keyDownInfo.pControl->getKbdRepeatable();
                });
    }
    // List containing keys which is currently pressed
    QList<KeyDownInformation> m_qActiveKeyList;
    // Pointer to keyboard config object
    ConfigObject<ConfigValueKbd> *m_pKbdConfigObject;
    // Multi-hash of key sequence to
    QMultiHash<ConfigValueKbd, ConfigKey> m_keySequenceToControlHash;
    // Learning mode flag
    bool m_learningMode;
    // Strict mode flag: consumes all unmapped keys
    bool m_strictMode;

    // Focused deck for deck-switching logic
    int m_focusedDeck; 
    void cycleFocusedDeck();
};
