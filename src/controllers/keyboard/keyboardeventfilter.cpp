#include <QList>
#include <QtDebug>
#include <QKeyEvent>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "control/controlobject.h"
#include "util/cmdlineargs.h"

KeyboardEventFilter::KeyboardEventFilter(QObject* parent, const char* name)
        : QObject(parent) {
    setObjectName(name);
}

KeyboardEventFilter::~KeyboardEventFilter() {
}

bool KeyboardEventFilter::eventFilter(QObject*, QEvent* e) {
    // TODO(Tomasito) Also clear active key list when keyboard controller disabled
    if (e->type() == QEvent::FocusOut) {
        // If we lose focus, we need to clear out the active key list
        // because we might not get Key Release events.
        m_qActiveKeyList.clear();
    } else if (e->type() == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent *)e;

#ifdef __APPLE__
        // On Mac OSX the nativeScanCode is empty (const 1) http://doc.qt.nokia.com/4.7/qkeyevent.html#nativeScanCode
        // We may loose the release event if a the shift key is pressed later
        // and there is character shift like "1" -> "!"
        int keyId = ke->key();
#else
        int keyId = ke->nativeScanCode();
#endif
        //qDebug() << "KeyPress event =" << ke->key() << "KeyId =" << keyId;

        // Run through list of active keys to see if the pressed key is already active
        // Just for returning true if we are consuming this key event

        foreach (const KeyDownInformation& keyDownInfo, m_qActiveKeyList) {
            if (keyDownInfo.keyId == keyId) {
                return true;
            }
        }

        QKeySequence ks = getKeySeq(ke);
        emit keySeqPressed(ks);

        // If no key-mapping info is known, there is nothing to check
        if (m_kbdPreset.isNull()) {
            return false;
        }

        if (!ks.isEmpty()) {
            ConfigValueKbd ksv(ks);

            // Check if a shortcut is defined
            bool result = false;

            QMultiHash<ConfigValueKbd, ConfigKey> mapping = m_kbdPreset->m_keySequenceToControlHash;
            QMultiHash<ConfigValueKbd, ConfigKey>::const_iterator iterator = mapping.find(ksv);

            // NOTE: Using const_iterator here is faster than QMultiHash::values()
            for (iterator; iterator != mapping.end(); ++iterator) {
                // TODO(Tomasito) Overload != operator for ConfigValueKbd and use it here
                if (!(iterator.key() == ksv)) continue;

                const ConfigKey& configKey = iterator.value();
                if (configKey.group == "[KeyboardShortcuts]") continue;

                ControlObject* control = ControlObject::getControl(configKey);

                // TODO(Tomasito) Move this to KeyboardController
                if (control) {
                    //qDebug() << configKey << "MIDI_NOTE_ON" << 1;
                    // Add key to active key list
                    m_qActiveKeyList.append(KeyDownInformation(
                        keyId, ke->modifiers(), control));
                    // Since setting the value might cause us to go down
                    // a route that would eventually clear the active
                    // key list, do that last.
                    control->setValueFromMidi(MIDI_NOTE_ON, 1);
                    result = true;
                } else {
                    qDebug() << "Warning: Keyboard key is configured for nonexistent control:"
                             << configKey.group << configKey.item;
                }

            }
            return result;
        }
    } else if (e->type()==QEvent::KeyRelease) {
        QKeyEvent* ke = (QKeyEvent*)e;

#ifdef __APPLE__
        // On Mac OSX the nativeScanCode is empty
        int keyId = ke->key();
#else
        int keyId = ke->nativeScanCode();
#endif
        bool autoRepeat = ke->isAutoRepeat();

        //qDebug() << "KeyRelease event =" << ke->key() << "AutoRepeat =" << autoRepeat << "KeyId =" << keyId;

        int clearModifiers = 0;
#ifdef __APPLE__
        // OS X apparently doesn't deliver KeyRelease events when you are
        // holding Ctrl. So release all key-presses that were triggered with
        // Ctrl.
        if (ke->key() == Qt::Key_Control) {
            clearModifiers = Qt::ControlModifier;
        }
#endif

        bool matched = false;
        // Run through list of active keys to see if the released key is active
        for (int i = m_qActiveKeyList.size() - 1; i >= 0; i--) {
            const KeyDownInformation& keyDownInfo = m_qActiveKeyList[i];
            ControlObject* pControl = keyDownInfo.pControl;
            if (keyDownInfo.keyId == keyId ||
                    (clearModifiers > 0 && keyDownInfo.modifiers == clearModifiers)) {
                if (!autoRepeat) {
                    //qDebug() << pControl->getKey() << "MIDI_NOTE_OFF" << 0;
                    pControl->setValueFromMidi(MIDI_NOTE_OFF, 0);
                    m_qActiveKeyList.removeAt(i);
                }
                // Due to the modifier clearing workaround we might match multiple keys for
                // release.
                matched = true;
            }
        }
        return matched;
    } else if (e->type() == QEvent::KeyboardLayoutChange) {
        // This event is not fired on ubunty natty, why?
        // TODO(XXX): find a way to support KeyboardLayoutChange Bug #997811
        //qDebug() << "QEvent::KeyboardLayoutChange";
    }
    return false;
}

QKeySequence KeyboardEventFilter::getKeySeq(QKeyEvent* e) {
    QString modseq;
    QKeySequence k;

    // TODO(XXX) check if we may simply return QKeySequence(e->modifiers()+e->key())

    if (e->modifiers() & Qt::ShiftModifier)
        modseq += "Shift+";

    if (e->modifiers() & Qt::ControlModifier)
        modseq += "Ctrl+";

    if (e->modifiers() & Qt::AltModifier)
        modseq += "Alt+";

    if (e->modifiers() & Qt::MetaModifier)
        modseq += "Meta+";

    if (e->key() >= 0x01000020 && e->key() <= 0x01000023) {
        // Do not act on Modifier only
        // avoid returning "khmer vowel sign ie (U+17C0)"
        return k;
    }

    QString keyseq = QKeySequence(e->key()).toString();
    k = QKeySequence(modseq + keyseq);

    if (CmdlineArgs::Instance().getDeveloper()) {
        qDebug() << "keyboard press: " << k.toString();
    }
    return k;
}

void KeyboardEventFilter::slotSetKeyboardMapping(ControllerPresetPointer presetPointer) {
    m_kbdPreset = presetPointer.dynamicCast<KeyboardControllerPreset>();

    // If preset pointer couldn't be casted back to KeyboardControllerPreset, the dynamic
    // cast returns null. That shouldn't happen.
    DEBUG_ASSERT(!m_kbdPreset.isNull());
}
