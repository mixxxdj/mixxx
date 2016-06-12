#ifndef CONTROLLERS_KEYBOARD_KEYBOARDEVENTFILTER_H
#define CONTROLLERS_KEYBOARD_KEYBOARDEVENTFILTER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QMultiHash>

#include "controllers/controllerpreset.h"
#include "preferences/configobject.h"
#include "keyboardcontrollerpreset.h"

class ControlObject;

// This class provides handling of keyboard events.
class KeyboardEventFilter : public QObject {
    Q_OBJECT
  public:
    KeyboardEventFilter(ConfigObject<ConfigValueKbd> *pKbdConfigObject,
                        QObject *parent = nullptr, const char* name = nullptr);
    virtual ~KeyboardEventFilter();

    bool eventFilter(QObject* obj, QEvent* e);

  signals:
    void keySeqPressed(QKeySequence keySeq);

  public slots:
    void slotSetKeyboardMapping(ControllerPresetPointer presetPointer);


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
    QKeySequence getKeySeq(QKeyEvent *e);

    // List containing keys which is currently pressed
    QList<KeyDownInformation> m_qActiveKeyList;

    // Multi-hash of key sequence to
    QMultiHash<ConfigValueKbd, ConfigKey> m_keySequenceToControlHash;

    // Clone of keyboard controller preset, containing keyboard mapping info
    QSharedPointer<KeyboardControllerPreset> m_kbdPreset;
};

#endif  // CONTROLLERS_KEYBOARD_KEYBOARDEVENTFILTER_H
