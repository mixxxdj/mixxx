#include "controllers/keyboard/keyboardeventfilter.h"

#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QtDebug>

#include "moc_keyboardeventfilter.cpp"
#include "util/cmdlineargs.h"
#include "util/logger.h"
#include "util/timer.h"
#include "widget/wbasewidget.h"

namespace {
const ConfigKey kKbdEnabledCfgKey =
        ConfigKey(QStringLiteral("[Keyboard]"), QStringLiteral("Enabled"));
mixxx::Logger kLogger("KeyboardEventFilter");

const QString mappingFilePath(const QString& dir, const QString& fileName) {
    return QDir(dir).filePath(fileName + QStringLiteral(".kbd.cfg"));
}
} // anonymous namespace

KeyboardEventFilter::KeyboardEventFilter(UserSettingsPointer pConfig,
        const QLocale& locale,
        QObject* parent)
        : QObject(parent),
#ifndef __APPLE__
          m_altPressedWithoutKey(false),
#endif
          m_pConfig(pConfig),
          m_locale(locale),
          m_enabled(false),
          m_autoReloader(RuntimeLoggingCategory(QStringLiteral("kbd_auto_reload"))) {
    // Get the enabled state.
    // Set the default if the key/value doesn't exist.
    if (pConfig->getValueString(kKbdEnabledCfgKey).isEmpty()) {
        pConfig->setValue(kKbdEnabledCfgKey, true);
    }
    m_enabled = pConfig->getValue(kKbdEnabledCfgKey, true);

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
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);

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
                if (configKey.group == "[KeyboardShortcuts]") {
                    // We don't handle menubar shortcuts here
                    continue;
                }
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
        // kLogger.debug() << "KeyRelease event =" << ke->key()
        // << "AutoRepeat=" << autoRepeat << "KeyId =" << keyId;

        int clearModifiers = Qt::NoModifier;
#ifdef __APPLE__
        // OS X apparently doesn't deliver KeyRelease events when you are
        // holding Ctrl. So release all key-presses that were triggered with
        // Ctrl.
        if (ke->key() == Qt::Key_Control) {
            clearModifiers = Qt::ControlModifier;
        }
#endif

        bool autoRepeat = ke->isAutoRepeat();
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
    m_pConfig->setValue(kKbdEnabledCfgKey, enabled);
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
    kLogger.debug() << "updateWidgetShortcuts";
    ScopedTimer timer(QStringLiteral("KeyboardEventFilter::updateWidgetShortcuts"));
    QStringList shortcutHints;
    for (auto* pWidget : std::as_const(m_widgets)) {
        shortcutHints.clear();
        QString keyString;
        const QList<std::pair<ConfigKey, QString>> controlsCommands =
                pWidget->getShortcutControlsAndCommands();
        for (const auto& [control, command] : controlsCommands) {
            keyString = m_pKbdConfig->getValueString(control);
            if (!keyString.isEmpty()) {
                shortcutHints.append(buildShortcutString(keyString, command));
            }
        }
        // might be empty to clear the previous tooltip
        pWidget->setShortcutTooltip(shortcutHints.join(QStringLiteral("\n")));
    }
    // Update widget tooltips (WBaseWidget handles no-ops).
    emit shortcutsEnabled(m_enabled);
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
    const QString nativeShortcut = QKeySequence(shortcut, QKeySequence::PortableText)
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
    m_menuBarActions.emplace(pAction, std::make_pair(command, defaultShortcut));
    pAction->setShortcut(QKeySequence(m_pKbdConfig->getValue(command, defaultShortcut)));
    pAction->setShortcutContext(Qt::ApplicationShortcut);
}

void KeyboardEventFilter::clearMenuBarActions() {
    m_menuBarActions.clear();
}

void KeyboardEventFilter::updateMenuBarActionShortcuts() {
    kLogger.debug() << "updateMenuBarActionShortcuts";
    QHashIterator<QAction*, std::pair<ConfigKey, QString>> it(m_menuBarActions);
    while (it.hasNext()) {
        it.next();
        auto* pAction = it.key();
        DEBUG_ASSERT(pAction);
        const QString keyStr = m_pKbdConfig->getValue(it.value().first, it.value().second);
        pAction->setShortcut(QKeySequence(keyStr));
    }
}

void KeyboardEventFilter::reloadKeyboardConfig() {
    kLogger.debug() << "reloadKeyboardConfig, enabled:" << m_enabled;
    ScopedTimer timer(QStringLiteral("KeyboardEventFilter::reload"));
    createKeyboardConfig();
    updateWidgetShortcuts();
    updateMenuBarActionShortcuts();
}

void KeyboardEventFilter::createKeyboardConfig() {
    kLogger.debug() << "createKeyboardConfig";
    // Remove the previously watched file.
    // Could be the user mapping has been removed and we'll need to switch
    // to the built-in default mapping.
    m_autoReloader.clear();

    // Check first in user's Mixxx directory
    QString keyboardFile = mappingFilePath(m_pConfig->getSettingsPath(), QStringLiteral("Custom"));
    if (QFile::exists(keyboardFile)) {
        kLogger.debug() << "Found and will use custom keyboard mapping" << keyboardFile;
    } else {
        // check if a default keyboard exists
        const QString resourcePath = m_pConfig->getResourcePath() + QStringLiteral("keyboard/");
        keyboardFile = mappingFilePath(resourcePath, m_locale.name());
        if (QFile::exists(keyboardFile)) {
            kLogger.debug() << "Found and will use default keyboard mapping" << keyboardFile;
        } else {
            kLogger.debug() << keyboardFile << " not found, try to use en_US.kbd.cfg";
            keyboardFile = mappingFilePath(resourcePath, QStringLiteral("en_US"));
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
    // Keyboard configs are a surjection from ConfigKey to key sequence.
    // We invert the mapping to create an injection from key sequence to
    // ConfigKey. This allows a key sequence to trigger multiple controls in
    // Mixxx.
    m_pKbdConfig = std::make_shared<ConfigObject<ConfigValueKbd>>(keyboardFile);
    // TODO Slightly accelerate lookup in eventFilter() by creating a copy
    // and removing [KeyboardShortcut] mappings (menubar) before transposing?
    m_keySequenceToControlHash = m_pKbdConfig->transpose();
}
