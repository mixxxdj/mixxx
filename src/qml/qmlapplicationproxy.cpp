#include "qml/qmlapplicationproxy.h"

#include <QDir>

#include "config.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "defs_urls.h"
#include "moc_qmlapplicationproxy.cpp"
#include "preferences/configobject.h"
#include "util/cmdlineargs.h"
#include "util/experiment.h"
#include "util/menubarhelper.h"
#include "util/versionstore.h"

namespace {
const ConfigKey kDebuggerEnabledConfigKey(
        QStringLiteral("[ScriptDebugger]"), QStringLiteral("Enabled"));
} // namespace

namespace mixxx {
namespace qml {

QmlApplicationProxy::QmlApplicationProxy(QObject* pParent)
        : QObject(pParent) {
    VERIFY_OR_DEBUG_ASSERT(s_pKeyboardEventFilter) {
        return;
    }
    connect(s_pKeyboardEventFilter.get(),
            &KeyboardEventFilter::shortcutsEnabled,
            this,
            &QmlApplicationProxy::keyboardShortcutsEnabledChanged);
    connect(s_pKeyboardEventFilter.get(),
            &KeyboardEventFilter::keyboardConfigReloaded,
            this,
            &QmlApplicationProxy::menuShortcutsChanged);
#if !defined(Q_OS_MACOS)
    connect(s_pKeyboardEventFilter.get(),
            &KeyboardEventFilter::altPressedWithoutKeys,
            this,
            &QmlApplicationProxy::applicationMenuRequested);
#endif
}

bool QmlApplicationProxy::keyboardShortcutsEnabled() const {
    return s_pKeyboardEventFilter && s_pKeyboardEventFilter->isEnabled();
}

void QmlApplicationProxy::setKeyboardShortcutsEnabled(bool enabled) {
    VERIFY_OR_DEBUG_ASSERT(s_pKeyboardEventFilter) {
        return;
    }
    if (s_pKeyboardEventFilter->isEnabled() == enabled) {
        return;
    }
    s_pKeyboardEventFilter->setEnabled(enabled);
}

bool QmlApplicationProxy::developerMode() const {
    return CmdlineArgs::Instance().getDeveloper();
}

bool QmlApplicationProxy::experimentStatsEnabled() const {
    return Experiment::isExperiment();
}

bool QmlApplicationProxy::baseStatsEnabled() const {
    return Experiment::isBase();
}

bool QmlApplicationProxy::debuggerEnabled() const {
    return s_pConfig && s_pConfig->getValue<bool>(kDebuggerEnabledConfigKey, false);
}

void QmlApplicationProxy::setDebuggerEnabled(bool enabled) {
    VERIFY_OR_DEBUG_ASSERT(s_pConfig) {
        return;
    }
    if (debuggerEnabled() == enabled) {
        return;
    }
    s_pConfig->setValue(kDebuggerEnabledConfigKey, enabled);
    emit debuggerEnabledChanged();
}

QUrl QmlApplicationProxy::settingsDirectoryUrl() const {
    return s_pConfig ? QUrl::fromLocalFile(s_pConfig->getSettingsPath()) : QUrl();
}

QString QmlApplicationProxy::applicationName() const {
    return VersionStore::applicationName();
}

QString QmlApplicationProxy::version() const {
    return VersionStore::version();
}

QString QmlApplicationProxy::platform() const {
    return VersionStore::platform();
}

bool QmlApplicationProxy::vinylControlAvailable() const {
#ifdef __VINYLCONTROL__
    return true;
#else
    return false;
#endif
}

bool QmlApplicationProxy::liveBroadcastingAvailable() const {
#ifdef __BROADCAST__
    return true;
#else
    return false;
#endif
}

QUrl QmlApplicationProxy::documentationUrl(
        const QString& fileName, const QString& onlineUrl) const {
    VERIFY_OR_DEBUG_ASSERT(s_pConfig) {
        return QUrl(onlineUrl);
    }
    QDir resourceDir(s_pConfig->getResourcePath());
#if defined(MIXXX_INSTALL_DOCDIR_RELATIVE_TO_DATADIR)
    if (!resourceDir.exists(fileName)) {
        resourceDir.cd(MIXXX_INSTALL_DOCDIR_RELATIVE_TO_DATADIR);
    }
#endif
    return resourceDir.exists(fileName)
            ? QUrl::fromLocalFile(resourceDir.absoluteFilePath(fileName))
            : QUrl(onlineUrl);
}

QUrl QmlApplicationProxy::userManualUrl() const {
    return documentationUrl(MIXXX_MANUAL_FILENAME, MIXXX_MANUAL_URL);
}

bool QmlApplicationProxy::userManualExternal() const {
    return !userManualUrl().isLocalFile();
}

QUrl QmlApplicationProxy::keyboardShortcutsUrl() const {
    return documentationUrl(MIXXX_KBD_SHORTCUTS_FILENAME, MIXXX_MANUAL_SHORTCUTS_URL);
}

bool QmlApplicationProxy::keyboardShortcutsExternal() const {
    return !keyboardShortcutsUrl().isLocalFile();
}

bool QmlApplicationProxy::supportsGlobalMenuBar() const {
    return desktopSupportsGlobalMenuBar();
}

void QmlApplicationProxy::setExperimentStatsEnabled(bool enabled) {
    if (enabled) {
        Experiment::setExperiment();
    } else {
        Experiment::disable();
    }
    emit statsModeChanged();
}

void QmlApplicationProxy::setBaseStatsEnabled(bool enabled) {
    if (enabled) {
        Experiment::setBase();
    } else {
        Experiment::disable();
    }
    emit statsModeChanged();
}

QString QmlApplicationProxy::menuShortcut(
        const QString& command, const QString& defaultShortcut) const {
    VERIFY_OR_DEBUG_ASSERT(s_pKeyboardEventFilter) {
        return defaultShortcut;
    }
    const auto pKeyboardConfig = s_pKeyboardEventFilter->getKeyboardConfig();
    VERIFY_OR_DEBUG_ASSERT(pKeyboardConfig) {
        return defaultShortcut;
    }
    return pKeyboardConfig->getValue(
            ConfigKey(QStringLiteral("[KeyboardShortcuts]"), command),
            defaultShortcut);
}

void QmlApplicationProxy::reloadSkin() {
    if (s_reloadCallback) {
        s_reloadCallback();
    }
}

QmlApplicationProxy* QmlApplicationProxy::create(
        QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    Q_UNUSED(pJsEngine);
    return new QmlApplicationProxy(pQmlEngine);
}

} // namespace qml
} // namespace mixxx
