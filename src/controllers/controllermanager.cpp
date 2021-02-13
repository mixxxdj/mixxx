#include "controllers/controllermanager.h"

#include <QSet>
#include <QThread>

#include "controllers/controllerlearningeventfilter.h"
#include "controllers/defs_controllers.h"
#include "controllers/midi/portmidienumerator.h"
#include "moc_controllermanager.cpp"
#include "util/cmdlineargs.h"
#include "util/time.h"
#include "util/trace.h"
#ifdef __HSS1394__
#include "controllers/midi/hss1394enumerator.h"
#endif

#ifdef __HID__
#include "controllers/hid/hidenumerator.h"
#endif

#ifdef __BULK__
#include "controllers/bulk/bulkenumerator.h"
#endif

// http://developer.qt.nokia.com/wiki/Threads_Events_QObjects

// Poll every 1ms (where possible) for good controller response
#ifdef __LINUX__
// Many Linux distros ship with the system tick set to 250Hz so 1ms timer
// reportedly causes CPU hosage. See Bug #990992 rryan 6/2012
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
          // ControllerManager because the CM is moved to its own thread and runs
          // its own event loop.
          m_pControllerLearningEventFilter(new ControllerLearningEventFilter()),
          m_pollTimer(this),
          m_skipPoll(false) {
    qRegisterMetaType<LegacyControllerMappingPointer>("LegacyControllerMappingPointer");

    // Create controller mapping paths in the user's home directory.
    QString userMappings = userMappingsPath(m_pConfig);
    if (!QDir(userMappings).exists()) {
        qDebug() << "Creating user controller mappings directory:" << userMappings;
        QDir().mkpath(userMappings);
    }

    m_pollTimer.setInterval(kPollInterval.toIntegerMillis());
    connect(&m_pollTimer, &QTimer::timeout, this, &ControllerManager::pollDevices);

    m_pThread = new QThread;
    m_pThread->setObjectName("Controller");

    // Moves all children (including the poll timer) to m_pThread
    moveToThread(m_pThread);

    // Controller processing needs to be prioritized since it can affect the
    // audio directly, like when scratching
    m_pThread->start(QThread::HighPriority);

    connect(this, &ControllerManager::requestInitialize, this, &ControllerManager::slotInitialize);
    connect(this,
            &ControllerManager::requestSetUpDevices,
            this,
            &ControllerManager::slotSetUpDevices);
    connect(this, &ControllerManager::requestShutdown, this, &ControllerManager::slotShutdown);

    // Signal that we should run slotInitialize once our event loop has started
    // up.
    emit requestInitialize(); // clazy:exclude=incorrect-emit
}

ControllerManager::~ControllerManager() {
    emit requestShutdown();
    m_pThread->wait();
    delete m_pThread;
    delete m_pControllerLearningEventFilter;
}

ControllerLearningEventFilter* ControllerManager::getControllerLearningEventFilter() const {
    return m_pControllerLearningEventFilter;
}

void ControllerManager::slotInitialize() {
    qDebug() << "ControllerManager:slotInitialize";

    // Initialize mapping info parsers. This object is only for use in the main
    // thread. Do not touch it from within ControllerManager.
    m_pMainThreadUserMappingEnumerator = QSharedPointer<MappingInfoEnumerator>(
            new MappingInfoEnumerator(userMappingsPath(m_pConfig)));
    m_pMainThreadSystemMappingEnumerator = QSharedPointer<MappingInfoEnumerator>(
            new MappingInfoEnumerator(resourceMappingsPath(m_pConfig)));

    // Instantiate all enumerators. Enumerators can take a long time to
    // construct since they interact with host MIDI APIs.
    m_enumerators.append(new PortMidiEnumerator());
#ifdef __HSS1394__
    m_enumerators.append(new Hss1394Enumerator(m_pConfig));
#endif
#ifdef __BULK__
    m_enumerators.append(new BulkEnumerator(m_pConfig));
#endif
#ifdef __HID__
    m_enumerators.append(new HidEnumerator());
#endif
}

void ControllerManager::slotShutdown() {
    stopPolling();

    // Clear m_enumerators before deleting the enumerators to prevent other code
    // paths from accessing them.
    QMutexLocker locker(&m_mutex);
    QList<ControllerEnumerator*> enumerators = m_enumerators;
    m_enumerators.clear();
    locker.unlock();

    // Delete enumerators and they'll delete their Devices
    for (ControllerEnumerator* pEnumerator : enumerators) {
        delete pEnumerator;
    }

    // Stop the processor after the enumerators since the engines live in it
    m_pThread->quit();
}

void ControllerManager::updateControllerList() {
    QMutexLocker locker(&m_mutex);
    if (m_enumerators.isEmpty()) {
        qWarning() << "updateControllerList called but no enumerators have been added!";
        return;
    }
    QList<ControllerEnumerator*> enumerators = m_enumerators;
    locker.unlock();

    QList<Controller*> newDeviceList;
    for (ControllerEnumerator* pEnumerator : enumerators) {
        newDeviceList.append(pEnumerator->queryDevices());
    }

    locker.relock();
    if (newDeviceList != m_controllers) {
        m_controllers = newDeviceList;
        locker.unlock();
        emit devicesChanged();
    }
}

QList<Controller*> ControllerManager::getControllers() const {
    QMutexLocker locker(&m_mutex);
    return m_controllers;
}

QList<Controller*> ControllerManager::getControllerList(bool bOutputDevices, bool bInputDevices) {
    qDebug() << "ControllerManager::getControllerList";

    QMutexLocker locker(&m_mutex);
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

QString ControllerManager::getConfiguredMappingFileForDevice(const QString& name) {
    return m_pConfig->getValueString(ConfigKey(kSettingsGroup, sanitizeDeviceName(name)));
}

void ControllerManager::slotSetUpDevices() {
    qDebug() << "ControllerManager: Setting up devices";

    updateControllerList();
    QList<Controller*> deviceList = getControllerList(false, true);
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

        LegacyControllerMappingPointer pMapping = LegacyControllerMappingFileHandler::loadMapping(
                mappingFile, resourceMappingsPath(m_pConfig));

        if (!pMapping) {
            continue;
        }

        pController->setMapping(*pMapping);

        // If we are in safe mode, skip opening controllers.
        if (CmdlineArgs::Instance().getSafeMode()) {
            qDebug() << "We are in safe mode -- skipping opening controller.";
            continue;
        }

        qDebug() << "Opening controller:" << name;

        int value = pController->open();
        if (value != 0) {
            qWarning() << "There was a problem opening" << name;
            continue;
        }
        pController->applyMapping();
    }

    maybeStartOrStopPolling();
}

void ControllerManager::maybeStartOrStopPolling() {
    QMutexLocker locker(&m_mutex);
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
    // Start the polling timer.
    if (!m_pollTimer.isActive()) {
        m_pollTimer.start();
        qDebug() << "Controller polling started.";
    }
}

void ControllerManager::stopPolling() {
    m_pollTimer.stop();
    qDebug() << "Controller polling stopped.";
}

void ControllerManager::pollDevices() {
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
    // Some random test data form a i5-3317U CPU @ 1.70GHz Running
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
    for (Controller* pDevice : qAsConst(m_controllers)) {
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
    if (!pController) {
        return;
    }
    if (pController->isOpen()) {
        pController->close();
    }
    int result = pController->open();
    maybeStartOrStopPolling();

    // If successfully opened the device, apply the mapping and save the
    // preference setting.
    if (result == 0) {
        pController->applyMapping();

        // Update configuration to reflect controller is enabled.
        m_pConfig->setValue(
                ConfigKey("[Controller]", sanitizeDeviceName(pController->getName())), 1);
    }
}

void ControllerManager::closeController(Controller* pController) {
    if (!pController) {
        return;
    }
    pController->close();
    maybeStartOrStopPolling();
    // Update configuration to reflect controller is disabled.
    m_pConfig->setValue(
            ConfigKey("[Controller]", sanitizeDeviceName(pController->getName())), 0);
}

void ControllerManager::slotApplyMapping(Controller* pController,
        LegacyControllerMappingPointer pMapping,
        bool bEnabled) {
    VERIFY_OR_DEBUG_ASSERT(pController) {
        qWarning() << "slotApplyMapping got invalid controller!";
        return;
    }

    ConfigKey key(kSettingsGroup, sanitizeDeviceName(pController->getName()));
    if (!pMapping) {
        closeController(pController);
        // Unset the controller mapping for this controller
        m_pConfig->remove(key);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(!pMapping->isDirty()) {
        qWarning() << "Mapping is dirty, changes might be lost on restart!";
    }

    pController->setMapping(*pMapping);

    // Save the file path/name in the config so it can be auto-loaded at
    // startup next time
    m_pConfig->set(key, pMapping->filePath());

    if (bEnabled) {
        openController(pController);
    } else {
        closeController(pController);
    }
}

// static
QList<QString> ControllerManager::getMappingPaths(UserSettingsPointer pConfig) {
    QList<QString> scriptPaths;
    scriptPaths.append(userMappingsPath(pConfig));
    scriptPaths.append(resourceMappingsPath(pConfig));
    return scriptPaths;
}
