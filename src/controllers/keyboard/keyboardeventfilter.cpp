#include "controllers/keyboard/keyboardeventfilter.h"

#include <QEvent>
#include <QKeyEvent>
#include <QtDebug>

#include "moc_keyboardeventfilter.cpp"
#include "util/cmdlineargs.h"

KeyboardEventFilter::KeyboardEventFilter(ConfigObject<ConfigValueKbd>* pKbdConfigObject,
        QObject* parent,
        const char* name)
        : QObject(parent),
#ifndef __APPLE__
          m_altPressedWithoutKey(false),
#endif
          m_pKbdConfigObject(nullptr),
          m_learningMode(false),
          m_strictMode(true), // Enabled by default for this "Strict Performance" suite
          m_focusedDeck(1) {
    setObjectName(name);
    setKeyboardConfig(pKbdConfigObject);
}

KeyboardEventFilter::~KeyboardEventFilter() {
}

bool KeyboardEventFilter::eventFilter(QObject*, QEvent* e) {
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

        if (shouldSkipHeldKey(keyId)) {
            return true;
        }

        QKeySequence ks = getKeySeq(ke);
        if (!ks.isEmpty()) {
            emit keyPressed(ks);

#ifndef __APPLE__
            m_altPressedWithoutKey = false;
#endif
            // If in learning mode, emit the captured key and don't trigger actions
            if (m_learningMode) {
                emit keyCaptured(ks);
                return true;
            }

            ConfigValueKbd ksv(ks);
            // Check if a shortcut is defined
            bool result = false;
            // using const_iterator here is faster than QMultiHash::values()
            for (auto it = m_keySequenceToControlHash.constFind(ksv);
                 it != m_keySequenceToControlHash.constEnd() && it.key() == ksv; ++it) {
                ConfigKey configKey = it.value();
                
                // Handle special [Keyboard] group for virtual actions
                if (configKey.group == "[Keyboard]") {
                    if (configKey.item == "cycle_focus_deck") {
                        cycleFocusedDeck();
                        result = true;
                        continue;
                    }
                }

                // Handle [ActiveDeck] redirection
                if (configKey.group == "[ActiveDeck]") {
                    configKey.group = QString("[Channel%1]").arg(m_focusedDeck);
                }

                QString context;
                
                // Parse context suffix if present (e.g., "play:Library")
                int colonIdx = configKey.item.lastIndexOf(':');
                if (colonIdx != -1) {
                    context = configKey.item.mid(colonIdx + 1);
                    configKey.item = configKey.item.left(colonIdx);
                }
                
                // Check context if specified
                if (!context.isEmpty()) {
                    QWidget* focusWidget = QApplication::focusWidget();
                    bool contextMatched = false;
                    
                    if (context == "Library") {
                        // Check if focus is in library
                        if (focusWidget && (focusWidget->objectName().contains("Library") || 
                            focusWidget->inherits("WLibraryTextFilter") || 
                            focusWidget->inherits("WTrackTableView"))) {
                            contextMatched = true;
                        }
                    } else if (context == "Deck") {
                        // Check if focus is on a deck component
                        if (focusWidget && (focusWidget->objectName().contains("Deck") || 
                            focusWidget->objectName().contains("Channel"))) {
                            contextMatched = true;
                        }
                    }
                    // Add more contexts as needed
                    
                    if (!contextMatched) continue;
                }

                if (configKey.group != "[KeyboardShortcuts]") {
                    ControlObject* control = ControlObject::getControl(configKey);
                    if (control) {
                        //qDebug() << configKey << "MidiOpCode::NoteOn" << 1;
                        // Add key to active key list
                        m_qActiveKeyList.append(KeyDownInformation(
                            keyId, ke->modifiers(), control));
                        // Since setting the value might cause us to go down
                        // a route that would eventually clear the active
                        // key list, do that last.
                        control->setValueFromMidi(MidiOpCode::NoteOn, 1);
                        result = true;
                    } else {
                        qDebug() << "Warning: Keyboard key is configured for nonexistent control:"
                                 << configKey.group << configKey.item;
                    }
                }
            }
            if (result) {
                return true;
            }

            // --- STRICT MODE ENFORCEMENT ---
            if (m_strictMode) {
                // If not consumed by a mapping, check if we should block it
                // from reaching default system handlers.
                QWidget* focusWidget = QApplication::focusWidget();
                bool isTextInput = focusWidget && (
                    focusWidget->inherits("QLineEdit") || 
                    focusWidget->inherits("QTextEdit") || 
                    focusWidget->inherits("QPlainTextEdit") ||
                    focusWidget->inherits("QComboBox") // ComboBox often has an edit field
                );

                if (!isTextInput) {
                    // Block the key to prevent fallback to system defaults
                    // (e.g., Space triggering Play even if unmapped)
                    return true;
                }
            }
            // -------------------------------
            
            return false;
#ifndef __APPLE__
        } else {
            // getKeySeq() returns empty string if the press was a modifier only
            // On most system Alt sends Alt + Qt::Key_Alt, but with Qt 6.9 (on Linux)
            // this changed apparently so that it's just Qt::Key_Alt
            if (((ke->modifiers() & Qt::AltModifier) || ke->key() == Qt::Key_Alt) &&
                    !m_altPressedWithoutKey) {
                m_altPressedWithoutKey = true;
            }
#endif
        }
    } else if (e->type() == QEvent::KeyRelease) {
        QKeyEvent* ke = (QKeyEvent*)e;

#ifndef __APPLE__
        // QAction hotkeys are consumed by the object that created them, e.g.
        // WMainMenuBar, so we will not receive menu hotkey keypress events here.
        // However, it may happen that we receive a RELEASE event for an Alt+key
        // combo for which no KEYPRESS was registered.
        // So react only to Alt-only releases.
        if (m_altPressedWithoutKey && ke->key() == Qt::Key_Alt) {
            emit altPressedWithoutKeys();
        }
        m_altPressedWithoutKey = false;
#endif

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
                    //qDebug() << pControl->getKey() << "MidiOpCode::NoteOff" << 0;
                    pControl->setValueFromMidi(MidiOpCode::NoteOff, 0);
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

// static
QKeySequence KeyboardEventFilter::getKeySeq(QKeyEvent* e) {
    if ((e->key() >= Qt::Key_Shift && e->key() <= Qt::Key_Alt) ||
            e->key() == Qt::Key_AltGr) {
        // Do not act on Modifier only, Shift, Ctrl, Meta, Alt and AltGr
        // avoid returning "khmer vowel sign ie (U+17C0)"
        return {};
    }

    // Note: test for individual modifiers, don't use e->modifiers() for composing
    // the QKeySequence because on macOS arrow key events are sent with the Num
    // modifier for some reason. This result in a key sequence for which there
    // would be no match in our keyseq/control hash.
    // See https://github.com/mixxxdj/mixxx/issues/13305
    QString modseq;
    if (e->modifiers() & Qt::ShiftModifier) {
        modseq += "Shift+";
    }
    if (e->modifiers() & Qt::ControlModifier) {
        modseq += "Ctrl+";
    }
    if (e->modifiers() & Qt::AltModifier) {
        modseq += "Alt+";
    }
    if (e->modifiers() & Qt::MetaModifier) {
        modseq += "Meta+";
    }

    const QString keyseq = QKeySequence(e->key()).toString();
    const QKeySequence k = QKeySequence(modseq + keyseq);

    if (CmdlineArgs::Instance().getDeveloper()) {
        if (e->type() == QEvent::KeyPress) {
            qDebug() << "keyboard press: " << k.toString();
        } else if (e->type() == QEvent::KeyRelease) {
            qDebug() << "keyboard release: " << k.toString();
        }
    }

    return k;
}

void KeyboardEventFilter::setKeyboardConfig(ConfigObject<ConfigValueKbd>* pKbdConfigObject) {
    // Keyboard configs are a surjection from ConfigKey to key sequence. We
    // invert the mapping to create an injection from key sequence to
    // ConfigKey. This allows a key sequence to trigger multiple controls in
    // Mixxx.
    m_keySequenceToControlHash = pKbdConfigObject->transpose();
    m_pKbdConfigObject = pKbdConfigObject;
}

ConfigObject<ConfigValueKbd>* KeyboardEventFilter::getKeyboardConfig() {
    return m_pKbdConfigObject;
}

void KeyboardEventFilter::setLearningMode(bool enable) {
    m_learningMode = enable;
    if (enable) {
        // Clear active keys when entering learning mode
        m_qActiveKeyList.clear();
    }
}
void KeyboardEventFilter::cycleFocusedDeck() {
    m_focusedDeck++;
    if (m_focusedDeck > 4) {
        m_focusedDeck = 1;
    }
    qDebug() << "Focused deck changed to:" << m_focusedDeck;
}
