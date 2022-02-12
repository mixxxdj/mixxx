#include "qmlapplication.h"

#include <QtQml/qqmlextensionplugin.h>

#include "control/controlsortfiltermodel.h"
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
Q_IMPORT_QML_PLUGIN(Mixxx_ControlsPlugin)

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
    QmlDlgPreferencesProxy::s_pInstance = new QmlDlgPreferencesProxy(m_pDlgPreferences, this);
    QmlEffectsManagerProxy::s_pInstance = new QmlEffectsManagerProxy(
            pCoreServices->getEffectsManager(), this);
    QmlPlayerManagerProxy::s_pInstance =
            new QmlPlayerManagerProxy(pCoreServices->getPlayerManager(), this);
    QmlConfigProxy::s_pInstance = new QmlConfigProxy(pCoreServices->getSettings(), this);
    QmlLibraryProxy::s_pInstance = new QmlLibraryProxy(pCoreServices->getLibrary(), this);

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
    m_pAppEngine->addImportPath(QStringLiteral(":/mixxx.org/imports"));

    // No memory leak here, the QQmlEngine takes ownership of the provider
    QQuickAsyncImageProvider* pImageProvider = new AsyncImageProvider(
            m_pCoreServices->getTrackCollectionManager());
    m_pAppEngine->addImageProvider(AsyncImageProvider::kProviderName, pImageProvider);

    m_pAppEngine->load(path);
    if (m_pAppEngine->rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML file" << path;
    }
}

} // namespace qml
} // namespace mixxx
