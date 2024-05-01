#include "controllers/keyboard/keyboardeventfilter.h"

#include <QEvent>
#include <QKeyEvent>
#include <QtDebug>

#include "moc_keyboardeventfilter.cpp"
#include "util/cmdlineargs.h"
#include "widget/wbasewidget.h"

KeyboardEventFilter::KeyboardEventFilter(UserSettingsPointer pConfig,
        QLocale& locale,
        QObject* parent,
        const char* name)
        : QObject(parent),
          m_pConfig(pConfig),
          m_locale(locale),
          m_enabled(false) {
    setObjectName(name);

    // get enabled state
    if (pConfig->getValueString(ConfigKey("[Keyboard]", "Enabled")).length() == 0) {
        pConfig->set(ConfigKey("[Keyboard]", "Enabled"), ConfigValue(1));
    }
    m_enabled = pConfig->getValue<bool>(ConfigKey("[Keyboard]", "Enabled"));

    createKeyboardConfig();

    // For watching the currently loaded mapping file
    connect(&m_fileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &KeyboardEventFilter::reloadKeyboardConfig);
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

        // If inactive, return after logging the key event in getKeySeq()
        if (!isEnabled()) {
            return true;
        }

        if (!ks.isEmpty()) {
            ConfigValueKbd ksv(ks);
            // Check if a shortcut is defined
            bool result = false;
            // using const_iterator here is faster than QMultiHash::values()
            for (auto it = m_keySequenceToControlHash.constFind(ksv);
                 it != m_keySequenceToControlHash.constEnd() && it.key() == ksv; ++it) {
                const ConfigKey& configKey = it.value();
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
            return result;
        }
    } else if (e->type() == QEvent::KeyRelease) {
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
    if (e->key() >= 0x01000020 && e->key() <= 0x01000023) {
        // Do not act on Modifier only, avoid returning "khmer vowel sign ie (U+17C0)"
        return {};
    }

    if (CmdlineArgs::Instance().getDeveloper()) {
        QString modseq;
        QKeySequence k;
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
        QString keyseq = QKeySequence(e->key()).toString();
        k = QKeySequence(modseq + keyseq);
        if (e->type() == QEvent::KeyPress) {
            qDebug() << "keyboard press: " << k.toString();
        } else if (e->type() == QEvent::KeyRelease) {
            qDebug() << "keyboard release: " << k.toString();
        }
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QKeySequence(e->modifiers() | e->key());
#else
    return QKeySequence(e->modifiers() + e->key());
#endif
}

void KeyboardEventFilter::setEnabled(bool enabled) {
    m_enabled = enabled;
    m_pConfig->set(ConfigKey("[Keyboard]", "Enabled"), ConfigValue(enabled));
    emit shortcutsEnabled(enabled);
}

void KeyboardEventFilter::registerShortcutWidget(WBaseWidget* pWidget) {
    m_widgets.append(pWidget);

    // Tell widgets to reconstruct tooltips when option is toggled.
    // WBaseWidget is not a QObject, need to use a lambda
    connect(this,
            &KeyboardEventFilter::shortcutsEnabled,
            this,
            [pWidget](bool enabled) {
                pWidget->toggleKeyboardShortcutHints(enabled);
            });
}

void KeyboardEventFilter::updateWidgetShortcuts() {
    for (auto pWidget : m_widgets) {
        QString shortcutHints;
        const QList<std::pair<ConfigKey, QString>> keys = pWidget->getShortcutKeys();
        QString keyString;
        for (const auto& key : keys) {
            keyString = m_pKbdConfig->getValueString(key.first);
            if (!keyString.isEmpty()) {
                shortcutHints.append(buildShortcutString(keyString, key.second));
            }
        }
        // might be empty
        pWidget->setShortcutTooltip(shortcutHints, m_enabled);
    }
}

void KeyboardEventFilter::clearWidgets() {
    disconnect(this, nullptr, nullptr, nullptr);
    m_widgets.clear();
}

const QString KeyboardEventFilter::buildShortcutString(
        const QString& shortcut, const QString& cmd) const {
    if (shortcut.isEmpty()) {
        return QString();
    }

    // translate shortcut to native text
    QString nativeShortcut = QKeySequence(shortcut, QKeySequence::PortableText)
                                     .toString(QKeySequence::NativeText);

    QString shortcutTooltip;
    shortcutTooltip += QObject::tr("Shortcut");
    if (!cmd.isEmpty()) {
        shortcutTooltip += " ";
        shortcutTooltip += cmd;
    }
    shortcutTooltip += ": ";
    shortcutTooltip += nativeShortcut;
    return shortcutTooltip;
}

void KeyboardEventFilter::reloadKeyboardConfig() {
    createKeyboardConfig();
    updateWidgetShortcuts();
}

void KeyboardEventFilter::createKeyboardConfig() {
    // Remove previously watched files.
    // Could be the user mapping was removed and we switch to the
    // built-in default mapping.
    const QStringList paths = m_fileWatcher.files();
    if (!paths.isEmpty()) {
        m_fileWatcher.removePaths(paths);
    }
    // Read keyboard configuration and set kdbConfig object in WWidget
    // Check first in user's Mixxx directory
    QString keyboardFile = QDir(m_pConfig->getSettingsPath()).filePath("Custom.kbd.cfg");
    if (QFile::exists(keyboardFile)) {
        qDebug() << "Found and will use custom keyboard mapping" << keyboardFile;
    } else {
        // check if a default keyboard exists
        QString resourcePath = m_pConfig->getResourcePath();
        keyboardFile = QString(resourcePath).append("keyboard/");
        keyboardFile += m_locale.name();
        keyboardFile += ".kbd.cfg";
        if (!QFile::exists(keyboardFile)) {
            qDebug() << keyboardFile << " not found, using en_US.kbd.cfg";
            keyboardFile = QString(resourcePath).append("keyboard/").append("en_US.kbd.cfg");
            if (!QFile::exists(keyboardFile)) {
                qDebug() << keyboardFile << " not found, starting without shortcuts";
                keyboardFile = "";
            }
        } else {
            qDebug() << "Found and will use default keyboard mapping" << keyboardFile;
        }
    }
    // Watch the loaded file for changes.
    m_fileWatcher.addPath(keyboardFile);

    // Keyboard configs are a surjection from ConfigKey to key sequence. We
    // invert the mapping to create an injection from key sequence to
    // ConfigKey. This allows a key sequence to trigger multiple controls in
    // Mixxx.
    m_pKbdConfig = std::make_shared<ConfigObject<ConfigValueKbd>>(keyboardFile);
    m_keySequenceToControlHash = m_pKbdConfig->transpose();
}

std::shared_ptr<ConfigObject<ConfigValueKbd>> KeyboardEventFilter::getKeyboardConfig() {
    return m_pKbdConfig;
}
