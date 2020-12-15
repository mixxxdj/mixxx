#include "qmlapplication.h"

#include "control/controlproxyqml.h"
#include "moc_qmlapplication.cpp"
#include "soundio/soundmanager.h"

namespace mixxx {

QmlApplication::QmlApplication(
        QApplication* app,
        std::shared_ptr<CoreServices> pCoreServices,
        const CmdlineArgs& args)
        : m_pCoreServices(pCoreServices),
          m_cmdlineArgs(args),
          m_pQmlAppEngine(nullptr),
          m_qmlFileWatcher({m_cmdlineArgs.getQmlPath()}) {
    qmlRegisterType<ControlProxyQml>("Mixxx", 1, 0, "Control");

    m_pCoreServices->initializeSettings();
    m_pCoreServices->initializeKeyboard();
    m_pCoreServices->initialize(app);
    SoundDeviceError result = m_pCoreServices->getSoundManager()->setupDevices();
    if (result != SOUNDDEVICE_ERROR_OK) {
        qCritical() << "Error setting up sound devices" << result;
        exit(result);
    }

    loadQml(m_cmdlineArgs.getQmlPath());

    connect(&m_qmlFileWatcher,
            &QFileSystemWatcher::fileChanged,
            this,
            &QmlApplication::loadQml);
}

void QmlApplication::loadQml(const QString& path) {
    // QQmlApplicationEngine::load creates a new window but also leaves the old one,
    // so it is necessary to destroy the old QQmlApplicationEngine and create a new one.
    m_pQmlAppEngine = std::make_unique<QQmlApplicationEngine>(path);
    if (m_pQmlAppEngine->rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML file" << path;
    }
}

} // namespace mixxx
