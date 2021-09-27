#include "qmlapplication.h"

#include "moc_qmlapplication.cpp"
#include "qml/asyncimageprovider.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmlcontrolproxy.h"
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

    qmlRegisterType<QmlControlProxy>("Mixxx", 0, 1, "ControlProxy");
    qmlRegisterType<QmlWaveformOverview>("Mixxx", 0, 1, "WaveformOverview");

    // Any uncreateable non-singleton types registered here require arguments
    // that we don't want to expose to QML directly. Instead, they can be
    // retrieved by member properties or methods from the singleton types.
    //
    // The alternative would be to register their *arguments* in the QML
    // system, which would improve nothing, or we had to expose them as
    // singletons to that they can be accessed by components instantiated by
    // QML, which would also be suboptimal.

    qmlRegisterSingletonType<QmlEffectsManagerProxy>("Mixxx",
            0,
            1,
            "EffectsManager",
            lambda_to_singleton_type_factory_ptr(
                    [pCoreServices](QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);

                        QmlEffectsManagerProxy* pEffectsManagerProxy =
                                new QmlEffectsManagerProxy(
                                        pCoreServices->getEffectsManager(),
                                        pEngine);
                        return pEffectsManagerProxy;
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
                    [pCoreServices](QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);

                        QmlPlayerManagerProxy* pPlayerManagerProxy =
                                new QmlPlayerManagerProxy(
                                        pCoreServices->getPlayerManager(),
                                        pEngine);
                        return pPlayerManagerProxy;
                    }));
    qmlRegisterUncreatableType<QmlPlayerProxy>("Mixxx",
            0,
            1,
            "Player",
            "Player objects can't be created directly, please use "
            "Mixxx.PlayerManager.getPlayer(group)");

    const auto pSettings = m_pCoreServices->getSettings();
    qmlRegisterSingletonType<QmlConfigProxy>("Mixxx",
            0,
            1,
            "Config",
            lambda_to_singleton_type_factory_ptr(
                    [pSettings](QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);

                        QmlConfigProxy* pConfigProxy =
                                new QmlConfigProxy(
                                        pSettings,
                                        pEngine);
                        return pConfigProxy;
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
                    [pCoreServices](QQmlEngine* pEngine,
                            QJSEngine* pScriptEngine) -> QObject* {
                        Q_UNUSED(pScriptEngine);

                        QmlLibraryProxy* pLibraryProxy = new QmlLibraryProxy(
                                pCoreServices->getLibrary(),
                                pEngine);
                        return pLibraryProxy;
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
