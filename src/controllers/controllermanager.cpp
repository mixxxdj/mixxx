#include "controllers/controllermanager.h"

#include <QSet>
#include <QThread>
#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>

#include "controllers/controller.h"
#include "controllers/controllerlearningeventfilter.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/defs_controllers.h"
#include "moc_controllermanager.cpp"
#include "util/cmdlineargs.h"
#include "util/compatibility/qmutex.h"
#include "util/logger.h"
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

namespace {

using namespace std::literals;

// Poll every 1ms (where possible) for good controller response
#ifdef __LINUX__
// Many Linux distros ship with the system tick set to 250Hz so 1ms timer
// reportedly causes CPU hosage. See Bug #990992 rryan 6/2012
constexpr auto kPollInterval = 5ms;
#else
constexpr auto kPollInterval = 1ms;
#endif

// Legacy code referred to mappings as "presets", so "[ControllerPreset]" must be
// kept for backwards compatibility.
const QString kSettingsGroup = QStringLiteral("[ControllerPreset]");

const mixxx::Logger kLogger("ControllerManager");

/// Strip slashes and spaces from device name, so that it can be used as config
/// key or a filename.
QString sanitizeDeviceName(QString name) {
    constexpr static QChar kReplacementChar = u'_';
    return name.replace(' ', kReplacementChar)
            .replace('/', kReplacementChar)
            .replace("\\", kReplacementChar);
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

} // anonymous namespace

bool controllerCompare(const Controller& lhs, const Controller& rhs) {
    return lhs.getName() < rhs.getName();
}

ControllerManager::ControllerManager(UserSettingsPointer pConfig)
        : QObject(),
          m_pConfig(pConfig),
          // WARNING: Do not parent m_pControllerLearningEventFilter to
          // ControllerManager because the CM is moved to its own thread and runs
          // its own event loop.
          m_pControllerLearningEventFilter(std::make_unique<ControllerLearningEventFilter>()),
          m_pollTimer(this),
          m_mutex(),
          m_enumerators(),
          m_controllers({}),
          m_pThread(nullptr),
          m_pMainThreadUserMappingEnumerator(nullptr),
          m_pMainThreadSystemMappingEnumerator(nullptr),
          m_skipPoll(false) {
    qRegisterMetaType<std::shared_ptr<LegacyControllerMapping>>(
            "std::shared_ptr<LegacyControllerMapping>");

    // Create controller mapping paths in the user's home directory.
    QString userMappings = userMappingsPath(m_pConfig);
    if (!QDir(userMappings).exists()) {
        kLogger.debug() << "Creating user controller mappings directory:" << userMappings;
        QDir().mkpath(userMappings);
    }

    m_pollTimer.setInterval(kPollInterval);
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
}

ControllerLearningEventFilter* ControllerManager::getControllerLearningEventFilter() const {
    return m_pControllerLearningEventFilter.get();
}

void ControllerManager::slotInitialize() {
    kLogger.debug() << "slotInitialize";

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
#ifdef __PORTMIDI__
    m_enumerators.push_back(std::make_unique<PortMidiEnumerator>());
#endif
#ifdef __HSS1394__
    m_enumerators.push_back(std::make_unique<Hss1394Enumerator>(m_pConfig));
#endif
#ifdef __BULK__
    m_enumerators.push_back(std::make_unique<BulkEnumerator>(m_pConfig));
#endif
#ifdef __HID__
    m_enumerators.push_back(std::make_unique<HidEnumerator>());
#endif
}

void ControllerManager::slotShutdown() {
    stopPolling();

    // Clear m_enumerators before deleting the enumerators to prevent other code
    // paths from accessing them.
    auto locker = lockMutex(&m_mutex);
    std::vector<std::unique_ptr<ControllerEnumerator>> enumerators = std::move(m_enumerators);
    locker.unlock();

    // Delete enumerators and they'll delete their Devices
    enumerators.clear();

    // Stop the processor after the enumerators since the engines live in it
    m_pThread->quit();
}

void ControllerManager::updateControllerList() {
    // NOTE: Currently this function is only called on startup. If hotplug is added, changes to the
    // controller list must be synchronized with dlgprefcontrollers to avoid dangling connections
    // and possible crashes.
    auto locker = lockMutex(&m_mutex);
    if (m_enumerators.empty()) {
        kLogger.warning() << "updateControllerList called but no enumerators have been added!";
        return;
    }
    std::vector<std::unique_ptr<ControllerEnumerator>> enumerators = std::move(m_enumerators);
    DEBUG_ASSERT(m_enumerators.empty());
    locker.unlock();

    QList<Controller*> newDeviceList;
    for (auto& pEnumerator : enumerators) {
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

QList<Controller*> ControllerManager::getControllerList(bool bOutputDevices, bool bInputDevices) {
    kLogger.debug() << "getControllerList";

    auto locker = lockMutex(&m_mutex);
    QList<Controller*> controllers = m_controllers;
    locker.unlock();

    // Create a list of controllers filtered to match the given input/output
    // options.
    QList<Controller*> filteredDeviceList;
    filteredDeviceList.reserve(controllers.size());
    std::copy_if(controllers.cbegin(),
            controllers.cend(),
            std::back_inserter(filteredDeviceList),
            [&](Controller* device) {
                return device->isOutputDevice() == bOutputDevices ||
                        device->isInputDevice() == bInputDevices;
            });
    return filteredDeviceList;
}

QString ControllerManager::getConfiguredMappingFileForDevice(const QString& name) {
    return m_pConfig->getValueString(ConfigKey(kSettingsGroup, sanitizeDeviceName(name)));
}

void ControllerManager::slotSetUpDevices() {
    kLogger.debug() << "Setting up devices";

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

        kLogger.debug() << "Searching for controller mapping" << mappingFilePath
                        << "in paths:" << mappingPaths.join(",");
        QFileInfo mappingFile = findMappingFile(mappingFilePath, mappingPaths);
        if (!mappingFile.exists()) {
            kLogger.debug() << "Could not find" << mappingFilePath << "in any mapping path.";
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
        pController->setMapping(pMapping->clone());

        // If we are in safe mode, skip opening controllers.
        if (CmdlineArgs::Instance().getSafeMode()) {
            kLogger.debug() << "We are in safe mode -- skipping opening controller.";
            continue;
        }

        kLogger.debug() << "Opening controller:" << name;

        int value = pController->open();
        if (value != 0) {
            kLogger.warning() << "There was a problem opening" << name;
            continue;
        }
        pController->applyMapping(m_pConfig->getResourcePath());
    }

    pollIfAnyControllersOpen();
}

void ControllerManager::pollIfAnyControllersOpen() {
    auto locker = lockMutex(&m_mutex);
    QList<Controller*> controllers = m_controllers;
    locker.unlock();

    const bool shouldPoll = std::any_of(controllers.cbegin(),
            controllers.cend(),
            [](Controller* pController) {
                return pController->isOpen() && pController->isPolling();
            });

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
        kLogger.debug() << "Controller polling started.";
    }
}

void ControllerManager::stopPolling() {
    m_pollTimer.stop();
    kLogger.debug() << "Controller polling stopped.";
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
        // kLogger.debug() << "ControllerManager::pollDevices() skip";
        return;
    }

    const auto start = mixxx::Time::now();
    for (Controller* pDevice : std::as_const(m_controllers)) {
        if (pDevice->isOpen() && pDevice->isPolling()) {
            pDevice->poll();
        }
    }

    std::chrono::nanoseconds duration = mixxx::Time::now() - start;
    if (duration > kPollInterval) {
        m_skipPoll = true;
    }
    // kLogger.debug() << "ControllerManager::pollDevices()" << duration << start;
}

void ControllerManager::openController(Controller* pController) {
    if (!pController) {
        return;
    }
    if (pController->isOpen()) {
        pController->close();
    }
    int result = pController->open();
    pollIfAnyControllersOpen();

    // If successfully opened the device, apply the mapping and save the
    // preference setting.
    if (result == 0) {
        pController->applyMapping(m_pConfig->getResourcePath());

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
    pollIfAnyControllersOpen();
    // Update configuration to reflect controller is disabled.
    m_pConfig->setValue(
            ConfigKey("[Controller]", sanitizeDeviceName(pController->getName())), 0);
}

void ControllerManager::slotApplyMapping(Controller* pController,
        std::shared_ptr<LegacyControllerMapping> pMapping,
        bool bEnabled) {
    VERIFY_OR_DEBUG_ASSERT(pController) {
        kLogger.warning() << "slotApplyMapping got invalid controller!";
        return;
    }

    ConfigKey key(kSettingsGroup, sanitizeDeviceName(pController->getName()));
    if (!pMapping) {
        closeController(pController);
        // Unset the controller mapping for this controller
        m_pConfig->remove(key);
        emit mappingApplied(false);
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(!pMapping->isDirty()) {
        kLogger.warning() << "Mapping is dirty, changes might be lost on restart!";
    }


    // Save the file path/name in the config so it can be auto-loaded at
    // startup next time
    m_pConfig->set(key, pMapping->filePath());

    // This runs on the main thread but LegacyControllerMapping is not thread safe, so clone it.
    pController->setMapping(pMapping->clone());

    if (bEnabled) {
        openController(pController);
        emit mappingApplied(pController->isMappable());
    } else {
        closeController(pController);
        emit mappingApplied(false);
    }
}

// static
QList<QString> ControllerManager::getMappingPaths(UserSettingsPointer pConfig) {
    return {userMappingsPath(pConfig), resourceMappingsPath(pConfig)};
}
