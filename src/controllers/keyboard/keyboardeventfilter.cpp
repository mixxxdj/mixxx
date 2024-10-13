#include "controllers/keyboard/keyboardeventfilter.h"

#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QtDebug>

#include "moc_keyboardeventfilter.cpp"
#include "util/cmdlineargs.h"
#include "util/logger.h"
#include "widget/wbasewidget.h"

namespace {
mixxx::Logger kLogger("AnalyzerThread");
} // anonymous namespace

KeyboardEventFilter::KeyboardEventFilter(UserSettingsPointer pConfig,
        QLocale& locale,
        QObject* parent,
        const char* name)
        : QObject(parent),
#ifndef __APPLE__
          m_altPressedWithoutKey(false),
#endif
          m_pConfig(pConfig),
          m_locale(locale),
          m_enabled(false),
          m_autoReloader(RuntimeLoggingCategory(QStringLiteral("kbd_auto_reload"))) {
    setObjectName(name);

    // get enabled state
    if (pConfig->getValueString(ConfigKey("[Keyboard]", "Enabled")).length() == 0) {
        pConfig->set(ConfigKey("[Keyboard]", "Enabled"), ConfigValue(1));
    }
    m_enabled = pConfig->getValue<bool>(ConfigKey("[Keyboard]", "Enabled"), true);

    createKeyboardConfig();

    // For watching the currently loaded mapping file
    connect(&m_autoReloader,
            &AutoFileReloader::fileChanged,
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
#ifndef __APPLE__
            m_altPressedWithoutKey = false;
#endif
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
                        // kLogger.debug() << configKey << "MidiOpCode::NoteOn" << 1;
                        // Add key to active key list
                        m_qActiveKeyList.append(KeyDownInformation(
                            keyId, ke->modifiers(), control));
                        // Since setting the value might cause us to go down
                        // a route that would eventually clear the active
                        // key list, do that last.
                        control->setValueFromMidi(MidiOpCode::NoteOn, 1);
                        result = true;
                    } else {
                        kLogger.warning() << "Key" << keyId
                                          << "is configured for nonexistent control:"
                                          << configKey.group << configKey.item;
                    }
                }
            }
            return result;
#ifndef __APPLE__
        } else {
            // getKeySeq() returns empty string if the press was a modifier only
            if ((ke->modifiers() & Qt::AltModifier) && !m_altPressedWithoutKey) {
                // on Linux pressing Alt sends Alt+Qt::Key_Alt, so checking for
                // Alt modifier is sufficient.
                // Activate this in case there are issues on Windows
                // || ke->key() == Qt::Key_Alt) {
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

        // kLogger.debug() << "KeyRelease event =" << ke->key()
        // << "AutoRepeat=" << autoRepeat << "KeyId =" << keyId;

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
                    // kLogger.debug() << pControl->getKey() << "MidiOpCode::NoteOff" << 0;
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
        // kLogger.debug() << "QEvent::KeyboardLayoutChange";
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
            kLogger.debug() << "keyboard press: " << k.toString();
        } else if (e->type() == QEvent::KeyRelease) {
            kLogger.debug() << "keyboard release: " << k.toString();
        }
    }

    return k;
}

void KeyboardEventFilter::setEnabled(bool enabled) {
    if (enabled) {
        kLogger.debug() << "Enable keyboard shortcuts/mappings";
    } else {
        kLogger.debug() << "Disable keyboard shortcuts/mappings";
    }
    m_enabled = enabled;
    m_pConfig->setValue(ConfigKey("[Keyboard]", "Enabled"), enabled);
    // Shortcuts may be toggled off and on again to make Mixxx discover a new
    // Custom.kbd.cfg, so reload now.
    // Note: the other way around (removing a loaded Custom.kbd.cfg) is covered
    // by the auto-reloader and we'll try to load a built-in mapping then.
    if (enabled) {
        reloadKeyboardConfig();
    }
    // Update widget tooltips
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
    for (auto* pWidget : m_widgets) {
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
    disconnect();
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
    shortcutTooltip += tr("Shortcut");
    if (!cmd.isEmpty()) {
        shortcutTooltip += " ";
        shortcutTooltip += cmd;
    }
    shortcutTooltip += ": ";
    shortcutTooltip += nativeShortcut;
    return shortcutTooltip;
}

void KeyboardEventFilter::registerMenuBarActionSetShortcut(QAction* pAction,
        const ConfigKey& command,
        const QString& defaultShortcut) {
    const auto cmdStr = std::make_pair(command, defaultShortcut);
    m_menuBarActions.insert(pAction, cmdStr);
    pAction->setShortcut(QKeySequence(m_pKbdConfig->getValue(command, defaultShortcut)));
    pAction->setShortcutContext(Qt::ApplicationShortcut);
}

void KeyboardEventFilter::clearMenuBarActions() {
    m_menuBarActions.clear();
}

void KeyboardEventFilter::updateMenuBarActionShortcuts() {
    QHashIterator<QAction*, std::pair<ConfigKey, QString>> it(m_menuBarActions);
    while (it.hasNext()) {
        it.next();
        const QString keyStr = m_pKbdConfig->getValue(it.value().first, it.value().second);
        auto* pAction = it.key();
        pAction->setShortcut(QKeySequence(keyStr));
    }
}

void KeyboardEventFilter::reloadKeyboardConfig() {
    createKeyboardConfig();
    updateWidgetShortcuts();
    updateMenuBarActionShortcuts();
}

void KeyboardEventFilter::createKeyboardConfig() {
    // Remove the previously watched file.
    // Could be the user mapping has been removed and we'll need to switch
    // to the built-in default mapping.
    m_autoReloader.clear();

    // Check first in user's Mixxx directory
    QString keyboardFile = QDir(m_pConfig->getSettingsPath()).filePath("Custom.kbd.cfg");
    if (QFile::exists(keyboardFile)) {
        kLogger.debug() << "Found and will use custom keyboard mapping" << keyboardFile;
    } else {
        // check if a default keyboard exists
        const QString resourcePath = m_pConfig->getResourcePath();
        keyboardFile = QString(resourcePath).append("keyboard/");
        keyboardFile += m_locale.name();
        keyboardFile += ".kbd.cfg";
        if (QFile::exists(keyboardFile)) {
            kLogger.debug() << "Found and will use default keyboard mapping" << keyboardFile;
        } else {
            kLogger.debug() << keyboardFile << " not found, using en_US.kbd.cfg";
            keyboardFile = QString(resourcePath).append("keyboard/").append("en_US.kbd.cfg");
            if (!QFile::exists(keyboardFile)) {
                kLogger.debug() << keyboardFile << " not found, starting without shortcuts";
                keyboardFile = "";
            }
        }
    }
    if (!keyboardFile.isEmpty()) {
        // Watch the loaded file for changes.
        m_autoReloader.addPath(keyboardFile);
    }

    // Read the keyboard configuration file and set m_pKbdConfig.
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
