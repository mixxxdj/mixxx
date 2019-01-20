#include <QList>
#include <QtDebug>
#include <QKeyEvent>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "util/cmdlineargs.h"

KeyboardEventFilter::KeyboardEventFilter(QObject* parent, const char* name)
        : QObject(parent),
          m_previousLayoutName(inputLocale().name()) {
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
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);

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
        for (const KeyDownInformation& keyDownInfo: m_qActiveKeyList) {
            if (keyDownInfo.keyId == keyId) {
                return true;
            }
        }

        if (CmdlineArgs::Instance().getDeveloper()) {
            qDebug() << "keyboard press: " << QKeySequence(ke->key()).toString();
        }

        QString ks = getKeySeq(ke);
        emit keyseqPressed(ks);

        // If no key-mapping info is known, there is nothing to check
        if (m_kbdPreset.isNull()) {
            return false;
        }

        if (!ks.isEmpty()) {
            // Check if a shortcut is defined
            bool result = false;

            // NOTE: Using const_iterator here is faster than QMultiHash::values()
            QMultiHash<QString, ConfigKey> mapping = m_kbdPreset->m_mapping;
            QMultiHash<QString, ConfigKey>::const_iterator iterator;
            for (iterator = mapping.find(ks); iterator != mapping.end(); ++iterator) {
                // NOTE: We are not breaking here because there could
                // potentially be another action mapped to the same
                // key sequence further down the hash table
                if (iterator.key() != ks) {
                    continue;
                }

                const ConfigKey& configKey = iterator.value();
                // These shortcuts are bound directly to QActions in WMainMenuBar::initialize()
                // and are updated in KeyboardShortcutsUpdater
                if (configKey.group == "[KeyboardShortcuts]") continue;

                ControlObject* control = ControlObject::getControl(configKey);
                if (control) {
                    // Add key to active key list
                    m_qActiveKeyList.append(KeyDownInformation(
                        keyId, ke->modifiers(), control));
                    emit controlKeySeqPressed(configKey);
                    result = true;
                } else {
                    qDebug() << "Warning: Keyboard key is configured for nonexistent control:"
                             << configKey.group << configKey.item;
                }
            }
            return result;
        }
    } else if (e->type()==QEvent::KeyRelease) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);

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

        // TODO(Tomasito) Event is not fired when Mixxx has no focus. So, if a user loses focus on Mixxx,
        // ...            changes the keyboard layout and gets back to Mixxx, the keyboard mapping will not
        // ...            be translated to his current locale: Add a check on focusInEvent (on MixxxMainWindow)

        QString layoutName = inputLocale().name();
        if (layoutName != m_previousLayoutName) {
            qDebug() << "QEvent::KeyboardLayoutChange" << layoutName;
            emit keyboardLayoutChanged(layoutName);
            m_previousLayoutName = layoutName;
        }
    }
    return false;
}

QString KeyboardEventFilter::getKeySeq(QKeyEvent* e) const {
    QString mods;
    QString keyText = e->text();

    // Return unicode text that was generated by this key-press if
    // no modifiers were applied to this key
    if (!e->modifiers() && !keyText.isEmpty()) {
        // Some keys do have a valid unicode position (keyText not empty), but would
        // be tricky to map on. Example: "Ctrl+ " for mapping to Ctrl+Space. In this
        // case we return a string representation of a QKeySequence, which will
        // return the key-name ("Tab", "Backspace" or "Space").
        int key = e->key();
        switch (key) {
            case Qt::Key_Tab:
            case Qt::Key_Backspace:
            case Qt::Key_Space:
                return QKeySequence(key).toString();
            default:
                break;
        }

        return keyText;
    }

    if (e->key() >= 0x01000020 && e->key() <= 0x01000023) {
        // Do not act on Modifier only
        // avoid returning "khmer vowel sign ie (U+17C0)"
        return "";
    }

    // TODO(XXX) check if we may simply return QKeySequence(e->modifiers()+e->key())

    // NOTE: This order must be the same as in
    // ...   KeyboardControllerPreset::translate()
    if (e->modifiers() & Qt::ShiftModifier)
        mods += "Shift+";
    if (e->modifiers() & Qt::ControlModifier)
        mods += "Ctrl+";
    if (e->modifiers() & Qt::AltModifier)
        mods += "Alt+";
    if (e->modifiers() & Qt::MetaModifier)
        mods += "Meta+";

    QString key = QKeySequence(e->key()).toString();
    return mods + key;
}
