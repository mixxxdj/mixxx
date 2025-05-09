#include "controllers/keyboard/keyboardeventfilter.h"

#include <QEvent>
#include <QKeyEvent>
#include <QtDebug>

#include "moc_keyboardeventfilter.cpp"
#include "util/cmdlineargs.h"

namespace {
const QString mappingFilePath(const QString& dir, const QString& fileName) {
    return QDir(dir).filePath(fileName + QStringLiteral(".kbd.cfg"));
}
} // anonymous namespace

KeyboardEventFilter::KeyboardEventFilter(UserSettingsPointer pConfig,
        const QLocale& locale,
        QObject* pParent)
        : QObject(pParent),
#ifndef __APPLE__
          m_altPressedWithoutKey(false),
#endif
          m_pConfig(pConfig),
          m_locale(locale),
          m_enabled(false) {
    // Get the enabled state.
    // Note: use the same default value in WMainMenuBar so that the action and
    // our bool are in sync.
    m_enabled = m_pConfig->getValue(ConfigKey(QStringLiteral("[Keyboard]"),
                                            QStringLiteral("Enabled")),
            true);

    createKeyboardConfig();
}

KeyboardEventFilter::~KeyboardEventFilter() {
}

bool KeyboardEventFilter::eventFilter(QObject*, QEvent* e) {
    if (e->type() == QEvent::FocusOut) {
        // If we lose focus, we need to clear out the active key list
        // because we might not get Key Release events.
        m_qActiveKeyList.clear();
    } else if (e->type() == QEvent::KeyPress) {
        QKeyEvent* pKE = static_cast<QKeyEvent*>(e);

#ifdef __APPLE__
        // On Mac OSX the nativeScanCode is empty (const 1) http://doc.qt.nokia.com/4.7/qkeyevent.html#nativeScanCode
        // We may loose the release event if a the shift key is pressed later
        // and there is character shift like "1" -> "!"
        int keyId = pKE->key();
#else
        int keyId = pKE->nativeScanCode();
#endif

        if (shouldSkipHeldKey(keyId)) {
            return true;
        }

        QKeySequence ks = getKeySeq(pKE);
        if (!ks.isEmpty()) {
#ifndef __APPLE__
            m_altPressedWithoutKey = false;
#endif
            // If inactive, return after logging the key event in getKeySeq()
            if (!isEnabled()) {
                return false;
            }

            ConfigValueKbd ksv(ks);
            // Check if a shortcut is defined
            bool result = false;
            // using const_iterator here is faster than QMultiHash::values()
            for (auto it = m_keySequenceToControlHash.constFind(ksv);
                 it != m_keySequenceToControlHash.constEnd() && it.key() == ksv; ++it) {
                const ConfigKey& configKey = it.value();

                if (configKey.group == QStringLiteral("[KeyboardShortcuts]")) {
                    // We don't handle menubar shortcuts here
                    continue;
                }

                ControlObject* pControl = ControlObject::getControl(configKey);
                if (pControl) {
                    // qDebug() << configKey << "MidiOpCode::NoteOn" << 1;
                    // Add key to active key list
                    m_qActiveKeyList.append(KeyDownInformation(
                            keyId, pKE->modifiers(), pControl));
                    // Since setting the value might cause us to go down
                    // a route that would eventually clear the active
                    // key list, do that last.
                    pControl->setValueFromMidi(MidiOpCode::NoteOn, 1);
                    result = true;
                } else {
                    qWarning() << "Key" << keyId
                               << "is configured for nonexistent control:"
                               << configKey.group << configKey.item;
                }
            }
            return result;
#ifndef __APPLE__
        } else {
            // getKeySeq() returns empty string if the press was a modifier only
            // On most system Alt sends Alt + Qt::Key_Alt, but with Qt 6.9 (on Linux)
            // this changed apparently so that it's just Qt::Key_Alt
            if (((pKE->modifiers() & Qt::AltModifier) || pKE->key() == Qt::Key_Alt) &&
                    !m_altPressedWithoutKey) {
                m_altPressedWithoutKey = true;
            }
#endif
        }
    } else if (e->type() == QEvent::KeyRelease) {
        QKeyEvent* pKE = static_cast<QKeyEvent*>(e);

#ifndef __APPLE__
        // QAction hotkeys are consumed by the object that created them, e.g.
        // WMainMenuBar, so we will not receive menu hotkey keypress events here.
        // However, it may happen that we receive a RELEASE event for an Alt+key
        // combo for which no KEYPRESS was registered.
        // So react only to Alt-only releases.
        if (m_altPressedWithoutKey && pKE->key() == Qt::Key_Alt) {
            emit altPressedWithoutKeys();
        }
        m_altPressedWithoutKey = false;
#endif

#ifdef __APPLE__
        // On Mac OSX the nativeScanCode is empty
        int keyId = pKE->key();
#else
        int keyId = pKE->nativeScanCode();
#endif

        //qDebug() << "KeyRelease event =" << ke->key() << "AutoRepeat =" << autoRepeat << "KeyId =" << keyId;

        Qt::KeyboardModifiers clearModifiers = Qt::NoModifier;
#ifdef __APPLE__
        // OS X apparently doesn't deliver KeyRelease events when you are
        // holding Ctrl. So release all key-presses that were triggered with
        // Ctrl.
        if (pKE->key() == Qt::Key_Control) {
            clearModifiers = Qt::ControlModifier;
        }
#endif

        bool autoRepeat = pKE->isAutoRepeat();
        bool matched = false;
        // Run through list of active keys to see if the released key is active.
        // Start from end because we may remove the current item.
        for (int i = m_qActiveKeyList.size() - 1; i >= 0; i--) {
            const KeyDownInformation& keyDownInfo = m_qActiveKeyList[i];
            ControlObject* pControl = keyDownInfo.pControl;
            if (keyDownInfo.keyId == keyId ||
                    (clearModifiers != Qt::NoModifier &&
                            keyDownInfo.modifiers == clearModifiers)) {
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
        // TODO: find a way to support KeyboardLayoutChange
        // https://github.com/mixxxdj/mixxx/issues/6424
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

void KeyboardEventFilter::setEnabled(bool enabled) {
    if (enabled) {
        qDebug() << "Enable keyboard shortcuts/mappings";
    } else {
        qDebug() << "Disable keyboard shortcuts/mappings";
    }
    m_enabled = enabled;
    m_pConfig->setValue(ConfigKey("[Keyboard]", "Enabled"), enabled);
}

void KeyboardEventFilter::createKeyboardConfig() {
    // Read keyboard configuration and set kdbConfig object in WWidget
    // Check first in user's Mixxx directory
    QString keyboardFile = mappingFilePath(m_pConfig->getSettingsPath(), QStringLiteral("Custom"));
    if (QFile::exists(keyboardFile)) {
        qDebug() << "Found and will use custom keyboard mapping" << keyboardFile;
    } else {
        // check if a default keyboard exists
        const QString resourcePath = m_pConfig->getResourcePath() + QStringLiteral("keyboard/");
        keyboardFile = mappingFilePath(resourcePath, m_locale.name());
        if (QFile::exists(keyboardFile)) {
            qDebug() << "Found and will use default keyboard mapping" << keyboardFile;
        } else {
            qDebug() << keyboardFile << " not found, try to use en_US.kbd.cfg";
            keyboardFile = mappingFilePath(resourcePath, QStringLiteral("en_US"));
            if (!QFile::exists(keyboardFile)) {
                qDebug() << keyboardFile << " not found, starting without shortcuts";
                keyboardFile = "";
            }
        }
    }

    // Read the keyboard configuration file and set m_pKbdConfig.
    // Keyboard configs are a surjection from ConfigKey to key sequence.
    // We invert the mapping to create an injection from key sequence to
    // ConfigKey. This allows a key sequence to trigger multiple controls in
    // Mixxx.
    m_pKbdConfig = std::make_shared<ConfigObject<ConfigValueKbd>>(keyboardFile);
    m_keySequenceToControlHash = m_pKbdConfig->transpose();
}
