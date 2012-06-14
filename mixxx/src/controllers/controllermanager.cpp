/**
  * @file controllermanager.cpp
  * @author Sean Pappalardo spappalardo@mixxx.org
  * @date Sat Apr 30 2011
  * @brief Manages creation/enumeration/deletion of hardware controllers.
  */

#include <QSet>

#include "util/sleepableqthread.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/controllerlearningeventfilter.h"

#include "controllers/midi/portmidienumerator.h"
#ifdef __HSS1394__
    #include "controllers/midi/hss1394enumerator.h"
#endif

#ifdef __HID__
    #include "controllers/hid/hidenumerator.h"
#endif

// http://developer.qt.nokia.com/wiki/Threads_Events_QObjects

// Poll every 1ms (where possible) for good controller response
#ifdef __LINUX__
// Many Linux distros ship with the system tick set to 250Hz so 1ms timer
// reportedly causes CPU hosage. See Bug #990992 rryan 6/2012
const int kPollIntervalMillis = 5;
#else
const int kPollIntervalMillis = 1;
#endif

QString firstAvailableFilename(QSet<QString>& filenames,
                               const QString originalFilename) {
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

ControllerManager::ControllerManager(ConfigObject<ConfigValue> * pConfig) :
        QObject(),
        m_pConfig(pConfig),
        // WARNING: Do not parent m_pControllerLearningEventFilter to
        // ControllerManager because the CM is moved to its own thread and runs
        // its own event loop.
        m_pControllerLearningEventFilter(new ControllerLearningEventFilter()),
        m_pollTimer(this) {

    qRegisterMetaType<ControllerPresetPointer>("ControllerPresetPointer");

    // Create controller mapping paths in the user's home directory.
    if (!QDir(USER_PRESETS_PATH).exists()) {
        qDebug() << "Creating user controller presets directory:" << USER_PRESETS_PATH;
        QDir().mkpath(USER_PRESETS_PATH);
    }
    if (!QDir(LOCAL_PRESETS_PATH).exists()) {
        qDebug() << "Creating local controller presets directory:" << LOCAL_PRESETS_PATH;
        QDir().mkpath(LOCAL_PRESETS_PATH);
    }

    // Initialize preset info parsers
    m_pPresetInfoManager = new PresetInfoEnumerator(m_pConfig);

    // Instantiate all enumerators
    m_enumerators.append(new PortMidiEnumerator());
#ifdef __HSS1394__
    m_enumerators.append(new Hss1394Enumerator());
#endif
#ifdef __HID__
    m_enumerators.append(new HidEnumerator());
#endif

    m_pollTimer.setInterval(kPollIntervalMillis);
    connect(&m_pollTimer, SIGNAL(timeout()),
            this, SLOT(pollDevices()));

    m_pThread = new QThread;
    m_pThread->setObjectName("Controller");

    // Moves all children (including the poll timer) to m_pThread
    moveToThread(m_pThread);

    // Controller processing needs to be prioritized since it can affect the
    // audio directly, like when scratching
    m_pThread->start(QThread::HighPriority);

    connect(this, SIGNAL(requestSetUpDevices()),
            this, SLOT(slotSetUpDevices()));
    connect(this, SIGNAL(requestShutdown()),
            this, SLOT(slotShutdown()));
    connect(this, SIGNAL(requestSave(bool)),
            this, SLOT(slotSavePresets(bool)));
}

ControllerManager::~ControllerManager() {
    m_pThread->wait();
    delete m_pThread;
    delete m_pControllerLearningEventFilter;
    delete m_pPresetInfoManager;
}

ControllerLearningEventFilter* ControllerManager::getControllerLearningEventFilter() const {
    return m_pControllerLearningEventFilter;
}

void ControllerManager::slotShutdown() {
    stopPolling();

    // Clear m_enumerators before deleting the enumerators to prevent other code
    // paths from accessing them.
    QMutexLocker locker(&m_mutex);
    QList<ControllerEnumerator*> enumerators = m_enumerators;
    m_enumerators.clear();
    m_mutex.unlock();

    // Delete enumerators and they'll delete their Devices
    foreach (ControllerEnumerator* pEnumerator, enumerators) {
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
    foreach (ControllerEnumerator* pEnumerator, enumerators) {
        newDeviceList.append(pEnumerator->queryDevices());
    }

    locker.relock();
    if (newDeviceList != m_controllers) {
        m_controllers = newDeviceList;
        locker.unlock();
        emit(devicesChanged());
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

    foreach (Controller* device, controllers) {
        if ((bOutputDevices == device->isOutputDevice()) ||
            (bInputDevices == device->isInputDevice())) {
            filteredDeviceList.push_back(device);
        }
    }
    return filteredDeviceList;
}

int ControllerManager::slotSetUpDevices() {
    qDebug() << "ControllerManager: Setting up devices";

    updateControllerList();
    QList<Controller*> deviceList = getControllerList(false, true);

    QSet<QString> filenames;
    int error = 0;

    foreach (Controller* pController, deviceList) {
        QString name = pController->getName();

        if (pController->isOpen()) {
            pController->close();
        }

        // The filename for this device name.
        const QString ofilename = presetFilenameFromName(name);
        // The first unique filename for this device (appends numbers at the end
        // if we have already seen a controller by this name on this run of
        // Mixxx.
        QString filename = firstAvailableFilename(filenames, ofilename);

        if (!loadPreset(pController, filename, true)) {
            // TODO(XXX) : auto load midi preset here.
            continue;
        }

        if (m_pConfig->getValueString(ConfigKey("[Controller]", ofilename)) != "1") {
            continue;
        }

        qDebug() << "Opening controller:" << name;

        int value = pController->open();
        if (value != 0) {
            qWarning() << "There was a problem opening" << name;
            if (error == 0) {
                error = value;
            }
            continue;
        }
        pController->applyPreset(m_pConfig->getConfigPath());
    }

    maybeStartOrStopPolling();
    return error;
}

void ControllerManager::maybeStartOrStopPolling() {
    QMutexLocker locker(&m_mutex);
    QList<Controller*> controllers = m_controllers;
    locker.unlock();

    bool shouldPoll = false;
    foreach (Controller* pController, controllers) {
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
    bool eventsProcessed(false);
    // Continue to poll while any device returned data.
    do {
        eventsProcessed = false;
        foreach (Controller* pDevice, m_controllers) {
            if (pDevice->isOpen() && pDevice->isPolling()) {
                eventsProcessed = pDevice->poll() || eventsProcessed;
            }
        }
    } while (eventsProcessed);
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

    // If successfully opened the device, apply the preset and save the
    // preference setting.
    if (result == 0) {
        pController->applyPreset(m_pConfig->getConfigPath());

        // Update configuration to reflect controller is enabled.
        m_pConfig->set(ConfigKey(
            "[Controller]", presetFilenameFromName(pController->getName())), 1);
    }
}

void ControllerManager::closeController(Controller* pController) {
    if (!pController) {
        return;
    }
    pController->close();
    maybeStartOrStopPolling();
    // Update configuration to reflect controller is disabled.
    m_pConfig->set(ConfigKey(
        "[Controller]", presetFilenameFromName(pController->getName())), 0);
}

bool ControllerManager::loadPreset(Controller* pController,
                                   ControllerPresetPointer preset) {
    if (!preset) {
        return false;
    }
    pController->setPreset(*preset.data());
    // Save the file path/name in the config so it can be auto-loaded at
    // startup next time
    m_pConfig->set(
        ConfigKey("[ControllerPreset]",
                  presetFilenameFromName(pController->getName())),
        preset->filePath());
    return true;
}

bool ControllerManager::loadPreset(Controller* pController,
                                   const QString &filename,
                                   const bool force) {
    QScopedPointer<ControllerPresetFileHandler> handler(pController->getFileHandler());
    if (!handler) {
        qWarning() << "Failed to get a file handler for" << pController->getName()
                   << " Unable to load preset.";
        return false;
    }

    // Handle case when filename is already valid full path to mapping
    // (coming from presetInfo.path)
    QString filenameWithExt;
    QString filepath;
    QFileInfo fileinfo(filename);
    if (fileinfo.isFile()) {
        filenameWithExt = fileinfo.baseName();
        filepath = fileinfo.absoluteFilePath();
    } else {
        filenameWithExt = filename + pController->presetExtension();
        filepath = USER_PRESETS_PATH + filenameWithExt;
    }

    // If the file isn't present in the user's directory, check the local
    // presets path.
    if (!QFile::exists(filepath)) {
        filepath = LOCAL_PRESETS_PATH.append(filenameWithExt);
    }

    // If the file isn't present in the user's directory, check res/
    if (!QFile::exists(filepath)) {
        filepath = m_pConfig->getConfigPath()
                .append("controllers/") + filenameWithExt;
    }

    if (!QFile::exists(filepath)) {
        qWarning() << "Cannot find" << filenameWithExt << "in either res/"
                   << "or the user's Mixxx directories (" + LOCAL_PRESETS_PATH
                   << USER_PRESETS_PATH + ")";
        return false;
    }

    ControllerPresetPointer pPreset = handler->load(
        filepath, filename, force);
    if (!pPreset) {
        qWarning() << "Unable to load preset" << filepath;
        return false;
    }

    loadPreset(pController, pPreset);
    //qDebug() << "Successfully loaded preset" << filepath;
    return true;
}

PresetInfoEnumerator* ControllerManager::getPresetInfoManager() {
    return m_pPresetInfoManager;
}

void ControllerManager::slotSavePresets(bool onlyActive) {
    QList<Controller*> deviceList = getControllerList(false, true);
    QSet<QString> filenames;

    foreach (Controller* pController, deviceList) {
        if (onlyActive && !pController->isOpen()) {
            continue;
        }
        QString name = pController->getName();
        QString filename = firstAvailableFilename(
            filenames, presetFilenameFromName(name));
        QString presetPath = USER_PRESETS_PATH + filename
                + pController->presetExtension();
        if (!pController->savePreset(presetPath)) {
            qWarning() << "Failed to write preset for device"
                       << name << "to" << presetPath;
        }
    }
}
