#pragma once

#include <QLocale>
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
    KeyboardEventFilter(UserSettingsPointer pConfig,
            const QLocale& locale,
            QObject* pParent = nullptr);
    virtual ~KeyboardEventFilter();

    bool eventFilter(QObject* obj, QEvent* e);

    std::shared_ptr<ConfigObject<ConfigValueKbd>> getKeyboardConfig() const {
        return m_pKbdConfig;
    };

    // Returns a valid QString with modifier keys from a QKeyEvent
    static QKeySequence getKeySeq(QKeyEvent* e);

    void setEnabled(bool enabled);
    bool isEnabled() {
        return m_enabled;
    }

#ifndef __APPLE__
  signals:
    void altPressedWithoutKeys();
#endif

  private:
    struct KeyDownInformation {
        KeyDownInformation(int keyId, Qt::KeyboardModifiers modifiers, ControlObject* pControl)
                : keyId(keyId),
                  modifiers(modifiers),
                  pControl(pControl) {
        }

        int keyId;
        Qt::KeyboardModifiers modifiers;
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

    void createKeyboardConfig();

    // List containing keys which is currently pressed
    QList<KeyDownInformation> m_qActiveKeyList;

    UserSettingsPointer m_pConfig;
    // Pointer to keyboard config object
    std::shared_ptr<ConfigObject<ConfigValueKbd>> m_pKbdConfig;
    QLocale m_locale;
    bool m_enabled;

    // Multi-hash of key sequence to
    QMultiHash<ConfigValueKbd, ConfigKey> m_keySequenceToControlHash;
};
