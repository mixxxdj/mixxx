#include "controllers/controllermanager.h"

#include <QSet>
#include <QThread>

#include "controllers/controller.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/defs_controllers.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "moc_controllermanager.cpp"
#include "preferences/usersettings.h"
#include "util/cmdlineargs.h"
#include "util/compatibility/qmutex.h"
#include "util/duration.h"
#include "util/thread_affinity.h"
#include "util/time.h"

#ifdef __PORTMIDI__
#include "controllers/midi/portmidienumerator.h"
#endif

#ifdef __HSS1394__
#include "controllers/midi/hss1394enumerator.h"
#endif

#ifdef __HID__
#include "controllers/hid/hidenumerator.h"
#endif

#ifdef __BULK__
#include "controllers/bulk/bulkenumerator.h"
#endif

#ifdef __ANDROID__
#include "controllers/midi/blemidienumerator.h"
#endif

// http://developer.qt.nokia.com/wiki/Threads_Events_QObjects

// Poll every 1ms (where possible) for good controller response
#ifdef __LINUX__
// Many Linux distros ship with the system tick set to 250Hz so 1ms timer
// reportedly causes CPU hosage. See https://github.com/mixxxdj/mixxx/issues/6383
// rryan 6/2012
const mixxx::Duration ControllerManager::kPollInterval = mixxx::Duration::fromMillis(5);
#else
const mixxx::Duration ControllerManager::kPollInterval = mixxx::Duration::fromMillis(1);
#endif

namespace {
/// Strip slashes and spaces from device name, so that it can be used as config
/// key or a filename.
QString sanitizeDeviceName(QString name) {
    return name.replace(" ", "_").replace("/", "_").replace("\\", "_");
}

QFileInfo findMappingFile(const QString& pathOrFilename, const QStringList& paths) {
    QFileInfo fileInfo(pathOrFilename);
    if (fileInfo.isAbsolute()) {
        return fileInfo;
    }

    for (const QString& path : paths) {
        fileInfo = QFileInfo(QDir(path).absoluteFilePath(pathOrFilename));
        if (fileInfo.exists()) {
            return fileInfo;
        }
    }

    return QFileInfo();
}

// Legacy code referred to mappings as "presets", so "[ControllerPreset]" must be
// kept for backwards compatibility.
const QString kSettingsGroup = QLatin1String("[ControllerPreset]");

} // anonymous namespace

QString firstAvailableFilename(QSet<QString>& filenames,
        const QString& originalFilename) {
    QString filename = originalFilename;
    int i = 1;
    while (filenames.contains(filename)) {
        i++;
        filename = QString("%1--%2").arg(originalFilename, QString::number(i));
    }
    filenames.insert(filename);
    return filename;
}

bool controllerCompare(Controller *a,Controller *b) {
    return a->getName() < b->getName();
}

ControllerManager::ControllerManager(UserSettingsPointer pConfig)
        : QObject(),
          m_pConfig(pConfig),
          // WARNING: Do not parent m_pControllerLearningEventFilter to
          // ControllerManager because the CM, together with all its children,
          // is moved to the ControllerManager thread (m_pThread) and
          // runs its own event loop there.
          m_pControllerLearningEventFilter(
                  std::make_unique<ControllerLearningEventFilter>()),
          m_pollTimer(this),
          m_pThread(std::make_unique<QThread>()),
          m_skipPoll(false) {
    qRegisterMetaType<std::shared_ptr<LegacyControllerMapping>>(
            "std::shared_ptr<LegacyControllerMapping>");

    // Create controller mapping paths in the user's home directory.
    QString userMappings = userMappingsPath(m_pConfig);
    if (!QDir(userMappings).exists()) {
        qDebug() << "Creating user controller mappings directory:" << userMappings;
        QDir().mkpath(userMappings);
    }

    m_pollTimer.setInterval(kPollInterval.toIntegerMillis());
    connect(&m_pollTimer, &QTimer::timeout, this, &ControllerManager::slotPollDevices);

    m_pThread->setObjectName("ControllerManager");

    // Move the entire ControllerManager object and all its children (including
    // the poll timer) onto the ControllerManager thread.
    moveToThread(m_pThread.get());

    // The ControllerManager thread is high-priority because controller input can
    // affect audio output directly (e.g. scratching).
    m_pThread->start(QThread::HighPriority);

    connect(this, &ControllerManager::requestInitialize, this, &ControllerManager::slotInitialize);
    connect(this,
            &ControllerManager::requestSetUpDevices,
            this,
            &ControllerManager::slotSetUpDevices);
    connect(this, &ControllerManager::requestShutdown, this, &ControllerManager::slotShutdown);

    // Signal that we should run slotInitialize once our event loop has started
    // up. invokeMethod with QueuedConnection will post an event to our event loop,
    // so slotInitialize will not run until after the ControllerManager thread is fully
    // started and ready to process events.
    QMetaObject::invokeMethod(this, &ControllerManager::slotInitialize, Qt::QueuedConnection);
}

ControllerManager::~ControllerManager() {
    // slotShutdown() closes controllers, deletes enumerators and calls
    // m_pThread->quit(). We must wait for the thread to finish before our
    // members (m_pollTimer, m_pControllerLearningEventFilter, …) are destroyed,
    // because they may still be accessed by the thread's event loop.
    emit requestShutdown(); // clazy:exclude=incorrect-emit
    m_pThread->wait();
    // m_pThread and m_pControllerLearningEventFilter are released by unique_ptr
    // after this point, in reverse declaration order.
}

ControllerLearningEventFilter* ControllerManager::getControllerLearningEventFilter() const {
    return m_pControllerLearningEventFilter.get();
}

void ControllerManager::slotInitialize() {
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    qDebug() << "ControllerManager:slotInitialize";

    // Initialize mapping info parsers. This object is only for use in the main
    // thread. Do not touch it from within ControllerManager.
    m_pMainThreadUserMappingEnumerator =
            QSharedPointer<MappingInfoEnumerator>::create(
                    userMappingsPath(m_pConfig));
    m_pMainThreadSystemMappingEnumerator =
            QSharedPointer<MappingInfoEnumerator>::create(
                    resourceMappingsPath(m_pConfig));

    // Instantiate all enumerators. Enumerators can take a long time to
    // construct since they interact with host MIDI APIs.
    {
        auto locker = lockMutex(&m_mutex);
#ifdef __PORTMIDI__
        m_enumerators.push_back(std::make_unique<PortMidiEnumerator>(m_pConfig));
#endif
#ifdef __HSS1394__
        m_enumerators.push_back(std::make_unique<Hss1394Enumerator>());
#endif
#ifdef __BULK__
        m_enumerators.push_back(std::make_unique<BulkEnumerator>());
#endif
#ifdef __HID__
        m_enumerators.push_back(std::make_unique<HidEnumerator>());
#endif
#ifdef __ANDROID__
        m_enumerators.push_back(std::make_unique<BleMidiEnumerator>(m_pConfig));
        // Keep raw pointer for BLE scan access (ownership is in m_enumerators)
        m_pBleScanEnumerator = static_cast<BleMidiEnumerator*>(m_enumerators.back().get());
#endif
    } // Mutex locker released here
#ifdef __ANDROID__
    // Start BLE scan early so devices are found before setUpDevices iterates
    startBleScan();
#endif
    emit initialized();
}

void ControllerManager::slotShutdown() {
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    stopPolling();

    // Clear m_enumerators before deleting the enumerators to prevent other code
    // paths from accessing them during teardown.
    auto locker = lockMutex(&m_mutex);
    std::vector<std::unique_ptr<ControllerEnumerator>> enumerators =
            std::move(m_enumerators); // m_enumerators is guaranteed empty after move
    locker.unlock();

    // Delete enumerators (and their owned Controllers) by letting unique_ptrs
    // go out of scope here — no raw deletes needed.
    enumerators.clear();

    // Stop the event loop after the enumerators are torn down, since the
    // controller scripting engines live inside the enumerators.
    m_pThread->quit();
}

void ControllerManager::updateControllerList() {
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    // NOTE: Currently this function is only called on startup. If hotplug is added, changes to the
    // controller list must be synchronized with dlgprefcontrollers to avoid dangling connections
    // and possible crashes.
    auto locker = lockMutex(&m_mutex);
    if (m_enumerators.empty()) {
        qWarning() << "updateControllerList called but no enumerators have been added!";
        return;
    }
    // Take a snapshot of the enumerator pointers while holding the lock.
    std::vector<ControllerEnumerator*> enumerators;
    enumerators.reserve(m_enumerators.size());
    for (const auto& pEnumerator : m_enumerators) {
        enumerators.push_back(pEnumerator.get());
    }
    locker.unlock();

    QList<Controller*> newDeviceList;
    for (ControllerEnumerator* pEnumerator : enumerators) {
        newDeviceList.append(pEnumerator->queryDevices());
    }

    locker.relock();
    if (newDeviceList == m_controllers) {
        return;
    }
    m_controllers = std::move(newDeviceList);
    locker.unlock();
    emit devicesChanged();
}

QList<Controller*> ControllerManager::getControllers() const {
    const auto locker = lockMutex(&m_mutex);
    return m_controllers;
}

QList<Controller*> ControllerManager::getControllerList(bool bOutputDevices, bool bInputDevices) {
    qDebug() << "ControllerManager::getControllerList";

    auto locker = lockMutex(&m_mutex);
    QList<Controller*> controllers = m_controllers;
    locker.unlock();

    // Create a list of controllers filtered to match the given input/output
    // options.
    QList<Controller*> filteredDeviceList;

    for (Controller* device : controllers) {
        if ((bOutputDevices == device->isOutputDevice()) ||
            (bInputDevices == device->isInputDevice())) {
            filteredDeviceList.push_back(device);
        }
    }
    return filteredDeviceList;
}

QString ControllerManager::getConfiguredMappingFileForDevice(const QString& name) const {
    // Thread-Safe : ConfigObject<ValueType>::getValueString/get is protected by QWriteLocker
    return m_pConfig->getValueString(ConfigKey(kSettingsGroup, sanitizeDeviceName(name)));
}

void ControllerManager::slotSetUpDevices() {
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    qDebug() << "ControllerManager: Setting up devices";

    updateControllerList();
    const QList<Controller*> deviceList = getControllerList(false, true);
    QStringList mappingPaths(getMappingPaths(m_pConfig));

    for (Controller* pController : deviceList) {
        QString name = pController->getName();

        if (pController->isOpen()) {
            pController->close();
        }

        // The filename for this device name.
        QString deviceName = sanitizeDeviceName(name);

        // Check if device is enabled
        if (!m_pConfig->getValue(ConfigKey("[Controller]", deviceName), 0)) {
            continue;
        }

        // Check if device has a configured mapping
        QString mappingFilePath = getConfiguredMappingFileForDevice(deviceName);
        if (mappingFilePath.isEmpty()) {
            continue;
        }

        qDebug() << "Searching for controller mapping" << mappingFilePath
                 << "in paths:" << mappingPaths.join(",");
        QFileInfo mappingFile = findMappingFile(mappingFilePath, mappingPaths);
        if (!mappingFile.exists()) {
            qDebug() << "Could not find" << mappingFilePath << "in any mapping path.";
            continue;
        }

        std::shared_ptr<LegacyControllerMapping> pMapping =
                LegacyControllerMappingFileHandler::loadMapping(
                        mappingFile, resourceMappingsPath(m_pConfig));

        if (!pMapping) {
            continue;
        }
        pMapping->loadSettings(m_pConfig, pController->getName());

        // This runs on the main thread but LegacyControllerMapping is not thread safe, so clone it.
        pController->setMapping(std::move(pMapping));

        // If we are in safe mode, skip opening controllers.
        if (CmdlineArgs::Instance().getSafeMode()) {
            qDebug() << "We are in safe mode -- skipping opening controller.";
            continue;
        }

        qDebug() << "Opening controller:" << name;

        int value = pController->open(m_pConfig->getResourcePath());
        if (value != 0) {
            qWarning() << "There was a problem opening" << name;
            continue;
        }
    }

    pollIfAnyControllersOpen();
}

void ControllerManager::pollIfAnyControllersOpen() {
    // Not Thread-Safe because calls startPolling()/stopPolling()
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    auto locker = lockMutex(&m_mutex);
    QList<Controller*> controllers = m_controllers;
    locker.unlock();

    bool shouldPoll = false;
    for (Controller* pController : controllers) {
        if (pController->isOpen() && pController->isPolling()) {
            shouldPoll = true;
        }
    }
    if (shouldPoll) {
        startPolling();
    } else {
        stopPolling();
    }
}

void ControllerManager::startPolling() {
    // Not Thread-Safe because QTimer::start() must be called from the thread that owns the timer.
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    // Start the polling timer.
    if (!m_pollTimer.isActive()) {
        m_pollTimer.start();
        qDebug() << "Controller polling started.";
    }
}

void ControllerManager::stopPolling() {
    // Not Thread-Safe because QTimer::stop() must be called from the thread that owns the timer.
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    m_pollTimer.stop();
    qDebug() << "Controller polling stopped.";
}

void ControllerManager::slotPollDevices() {
    // Not Thread-Safe because it accesses m_controllers and m_skipPoll without a mutex.
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    // Note: this function is called from a high priority thread which
    // may stall the GUI or may reduce the available CPU time for other
    // High Priority threads like caching reader or broadcasting more
    // then desired, if it is called endless loop like.
    //
    // This especially happens if a controller like the 3x Speed
    // Stanton SCS.1D emits more massages than Mixxx is able to handle
    // or a controller like Hercules RMX2 goes wild. In such a case the
    // receive buffer is stacked up every call to insane values > 500 messages.
    //
    // To avoid this we pick here a strategies similar like the audio
    // thread. In case pollDevice() takes longer than a call cycle
    // we are cooperative a skip the next cycle to free at least some
    // CPU time
    //
    // Some random test data from a i5-3317U CPU @ 1.70GHz Running
    // Ubuntu Trusty:
    // * Idle poll: ~5 µs.
    // * 5 messages burst (full midi bandwidth): ~872 µs.

    if (m_skipPoll) {
        // skip poll in overload situation
        m_skipPoll = false;
        //qDebug() << "ControllerManager::pollDevices() skip";
        return;
    }

    mixxx::Duration start = mixxx::Time::elapsed();
    for (Controller* pDevice : std::as_const(m_controllers)) {
        if (pDevice->isOpen() && pDevice->isPolling()) {
            pDevice->poll();
        }
    }

    mixxx::Duration duration = mixxx::Time::elapsed() - start;
    if (duration > kPollInterval) {
        m_skipPoll = true;
    }
    //qDebug() << "ControllerManager::pollDevices()" << duration << start;
}

void ControllerManager::openController(Controller* pController) {
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    if (!pController) {
        return;
    }
    if (pController->isOpen()) {
        pController->close();
    }
    int result = pController->open(m_pConfig->getResourcePath());
    pollIfAnyControllersOpen();

    // If successfully opened the device, apply the mapping and save the
    // preference setting.
    if (result == 0) {
        // Update configuration to reflect controller is enabled.
        m_pConfig->setValue(
                ConfigKey("[Controller]", sanitizeDeviceName(pController->getName())), 1);
    }
}

void ControllerManager::closeController(Controller* pController) {
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    if (!pController) {
        return;
    }
    pController->close();
    pollIfAnyControllersOpen();
    // Update configuration to reflect controller is disabled.
    m_pConfig->setValue(
            ConfigKey("[Controller]", sanitizeDeviceName(pController->getName())), 0);
}

// This needs to be called in a Qt::BlockingQueuedConnection so that the
// signaling thread can't alter the LegacyControllerMapping during applying
void ControllerManager::slotApplyMapping(Controller* pController,
        std::shared_ptr<LegacyControllerMapping> pMapping,
        bool bEnabled) {
    DEBUG_ASSERT_THIS_QOBJECT_THREAD_AFFINITY();
    VERIFY_OR_DEBUG_ASSERT(pController) {
        qWarning() << "slotApplyMapping got invalid controller!";
        return;
    }

    closeController(pController);
    ConfigKey key(kSettingsGroup, sanitizeDeviceName(pController->getName()));
    if (!pMapping) {
        // Unset the controller mapping for this controller
        pController->setMapping(nullptr);
        m_pConfig->remove(key);
        emit mappingApplied(false);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(!pMapping->isDirty()) {
        qWarning() << "Mapping is dirty, changes might be lost on restart!";
    }

    // Save the file path/name in the config so it can be auto-loaded at
    // startup next time
    m_pConfig->set(key, pMapping->filePath());

    pController->setMapping(std::move(pMapping));

    if (bEnabled) {
        emit mappingApplied(pController->isMappable());
    } else {
        emit mappingApplied(false);
        return;
    }

    // Note: openController() may call ControllerRenderingEngine::setup()
    // Which has a blocking invokeMethod() call for QOffscreenSurface::create()
    // That why we need to return from this blocking call first.
    QMetaObject::invokeMethod(
            this,
            [this, pController]() { openController(pController); },
            Qt::QueuedConnection);
}

// static
QList<QString> ControllerManager::getMappingPaths(UserSettingsPointer pConfig) {
    QList<QString> scriptPaths;
    scriptPaths.append(userMappingsPath(pConfig));
    scriptPaths.append(resourceMappingsPath(pConfig));
    return scriptPaths;
}

#ifdef __ANDROID__
void ControllerManager::startBleScan() {
    if (m_pBleScanEnumerator) {
        m_pBleScanEnumerator->startScan();
    }
}

bool ControllerManager::isBleConnected() const {
    if (m_pBleScanEnumerator) {
        return m_pBleScanEnumerator->isConnected();
    }
    return false;
}
#endif
