#ifndef CONTROLLERS_KEYBOARD_KEYBOARDEVENTFILTER_H
#define CONTROLLERS_KEYBOARD_KEYBOARDEVENTFILTER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QMultiHash>

#include "controllers/keyboard/keyboardcontrollerpreset.h"

class ControlObject;

// This class provides handling of keyboard events.
class KeyboardEventFilter : public QObject {
    Q_OBJECT
  public:
    KeyboardEventFilter(QObject* parent = nullptr, const char* name = nullptr);
    virtual ~KeyboardEventFilter();
    bool eventFilter(QObject* obj, QEvent* e);

  public slots:
    void slotSetKeyboardMapping(KeyboardControllerPresetPointer presetPointer) {
        m_kbdPreset = presetPointer;
    };

  signals:
    void keySeqPressed(QKeySequence keySeq);
    void controlKeySeqPressed(ConfigKey configKey);

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

    // Returns a valid QString with modifier keys from a QKeyEvent
    QString getKeySeq(QKeyEvent *e);

    // List containing keys which is currently pressed
    QList<KeyDownInformation> m_qActiveKeyList;

    // Clone of keyboard controller preset, containing keyboard mapping info
    QSharedPointer<KeyboardControllerPreset> m_kbdPreset;
};

#endif  // CONTROLLERS_KEYBOARD_KEYBOARDEVENTFILTER_H
