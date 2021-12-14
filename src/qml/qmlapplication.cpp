#include "qmlapplication.h"

#include "control/controlmodel.h"
#include "moc_qmlapplication.cpp"
#include "qml/asyncimageprovider.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmlcontrolproxy.h"
#include "qml/qmldlgpreferencesproxy.h"
#include "qml/qmleffectmanifestparametersmodel.h"
#include "qml/qmleffectslotproxy.h"
#include "qml/qmleffectsmanagerproxy.h"
#include "qml/qmllibraryproxy.h"
#include "qml/qmllibrarytracklistmodel.h"
#include "qml/qmlplayermanagerproxy.h"
#include "qml/qmlplayerproxy.h"
#include "qml/qmlvisibleeffectsmodel.h"
#include "qml/qmlwaveformoverview.h"
#include "soundio/soundmanager.h"

namespace {
const QString kMainQmlFileName = QStringLiteral("qml/main.qml");

// Converts a (capturing) lambda into a function pointer that can be passed to
// qmlRegisterSingletonType.
template<class F>
auto lambda_to_singleton_type_factory_ptr(F&& f) {
    static F fn = std::forward<F>(f);
    return [](QQmlEngine* pEngine, QJSEngine* pScriptEngine) -> QObject* {
        return fn(pEngine, pScriptEngine);
    };
}
} // namespace

namespace mixxx {
namespace qml {

QmlApplication::QmlApplication(
        QApplication* app,
        std::shared_ptr<CoreServices> pCoreServices)
        : m_pCoreServices(pCoreServices),
          m_mainFilePath(pCoreServices->getSettings()->getResourcePath() + kMainQmlFileName),
          m_pAppEngine(nullptr),
          m_fileWatcher({m_mainFilePath}) {
    m_pCoreServices->initialize(app);
    SoundDeviceError result = m_pCoreServices->getSoundManager()->setupDevices();
    if (result != SOUNDDEVICE_ERROR_OK) {
        qCritical() << "Error setting up sound devices" << result;
        exit(result);
    }

    // FIXME: DlgPreferences has some initialization logic that must be executed
    // before the GUI is shown, at least for the effects system.
    m_pDlgPreferences = std::make_shared<DlgPreferences>(
            m_pCoreServices->getScreensaverManager(),
            nullptr,
            m_pCoreServices->getSoundManager(),
            m_pCoreServices->getControllerManager(),
            m_pCoreServices->getVinylControlManager(),
            m_pCoreServices->getEffectsManager(),
            m_pCoreServices->getSettingsManager(),
            m_pCoreServices->getLibrary());
    // Without this, QApplication will quit when the last QWidget QWindow is
    // closed because it does not take into account the window created by
    // the QQmlApplicationEngine.
    m_pDlgPreferences->setAttribute(Qt::WA_QuitOnClose, false);

    // Any uncreateable non-singleton types registered here require arguments
    // that we don't want to expose to QML directly. Instead, they can be
    // retrieved by member properties or methods from the singleton types.
    //
    // The alternative would be to register their *arguments* in the QML
    // system, which would improve nothing, or we had to expose them as
    // singletons to that they can be accessed by components instantiated by
    // QML, which would also be suboptimal.
    //
    // WARNING: The lambdas passed to qmlRegisterSingletonType outlive this
    // QmlApplication. Therefore they *must not* capture any pointers which
    // they share ownership of. Otherwise, the values captured by the lambdas
    // would be destroyed after CoreServices and after the Qt event loop stops
    // processing. To work around this, pass weak_ptrs into the lambdas and
    // convert them to shared_ptrs when the QQmlApplicationEngine instantiates
    // the QML proxy objects.

    qmlRegisterSingletonType<QmlDlgPreferencesProxy>("Mixxx",
            0,
            1,
            "PreferencesDialog",
            lambda_to_singleton_type_factory_ptr(
                    [pDlgPreferences = std::weak_ptr<DlgPreferences>(
                             m_pDlgPreferences)](QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);
                        return new QmlDlgPreferencesProxy(pDlgPreferences.lock(), pEngine);
                    }));

    qmlRegisterType<QmlControlProxy>("Mixxx", 0, 1, "ControlProxy");
    qmlRegisterType<ControlModel>("Mixxx", 0, 1, "ControlModel");
    qmlRegisterType<QmlWaveformOverview>("Mixxx", 0, 1, "WaveformOverview");

    qmlRegisterSingletonType<QmlEffectsManagerProxy>("Mixxx",
            0,
            1,
            "EffectsManager",
            lambda_to_singleton_type_factory_ptr(
                    [pEffectsManager = std::weak_ptr<EffectsManager>(
                             pCoreServices->getEffectsManager())](
                            QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);
                        return new QmlEffectsManagerProxy(pEffectsManager.lock(), pEngine);
                    }));
    qmlRegisterUncreatableType<QmlVisibleEffectsModel>("Mixxx",
            0,
            1,
            "VisibleEffectsModel",
            "VisibleEffectsModel objects can't be created directly, please use "
            "Mixxx.EffectsManager.visibleEffectsModel");
    qmlRegisterUncreatableType<QmlEffectManifestParametersModel>("Mixxx",
            0,
            1,
            "EffectManifestParametersModel",
            "EffectManifestParametersModel objects can't be created directly, "
            "please use Mixxx.EffectsSlot.parametersModel");
    qmlRegisterUncreatableType<QmlEffectSlotProxy>("Mixxx",
            0,
            1,
            "EffectSlotProxy",
            "EffectSlotProxy objects can't be created directly, please use "
            "Mixxx.EffectsManager.getEffectSlot(rackNumber, unitNumber, effectNumber)");

    qmlRegisterSingletonType<QmlPlayerManagerProxy>("Mixxx",
            0,
            1,
            "PlayerManager",
            lambda_to_singleton_type_factory_ptr(
                    [pPlayerManager = std::weak_ptr<PlayerManager>(
                             pCoreServices->getPlayerManager())](
                            QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);
                        return new QmlPlayerManagerProxy(pPlayerManager.lock(), pEngine);
                    }));
    qmlRegisterUncreatableType<QmlPlayerProxy>("Mixxx",
            0,
            1,
            "Player",
            "Player objects can't be created directly, please use "
            "Mixxx.PlayerManager.getPlayer(group)");

    qmlRegisterSingletonType<QmlConfigProxy>("Mixxx",
            0,
            1,
            "Config",
            lambda_to_singleton_type_factory_ptr(
                    [pSettings = QWeakPointer(pCoreServices->getSettings())](
                            QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);
                        return new QmlConfigProxy(pSettings, pEngine);
                    }));

    qmlRegisterUncreatableType<QmlLibraryTrackListModel>("Mixxx",
            0,
            1,
            "LibraryTrackListModel",
            "LibraryTrackListModel objects can't be created directly, "
            "please use Mixxx.Library.model");
    qmlRegisterSingletonType<QmlLibraryProxy>("Mixxx",
            0,
            1,
            "Library",
            lambda_to_singleton_type_factory_ptr(
                    [pLibrary = std::weak_ptr<Library>(
                             pCoreServices->getLibrary())](QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);
                        return new QmlLibraryProxy(pLibrary.lock(), pEngine);
                    }));

    loadQml(m_mainFilePath);

    connect(&m_fileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &QmlApplication::loadQml);
}

void QmlApplication::loadQml(const QString& path) {
    // QQmlApplicationEngine::load creates a new window but also leaves the old one,
    // so it is necessary to destroy the old QQmlApplicationEngine and create a new one.
    m_pAppEngine = std::make_unique<QQmlApplicationEngine>();

    const QFileInfo fileInfo(path);
    m_pAppEngine->addImportPath(fileInfo.absoluteDir().absolutePath());

    // No memory leak here, the QQmlEngine takes ownership of the provider
    QQuickAsyncImageProvider* pImageProvider = new AsyncImageProvider();
    m_pAppEngine->addImageProvider(AsyncImageProvider::kProviderName, pImageProvider);

    m_pAppEngine->load(path);
    if (m_pAppEngine->rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML file" << path;
    }
}

} // namespace qml
} // namespace mixxx
