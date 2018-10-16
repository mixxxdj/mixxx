/**
  * @file controllermanager.cpp
  * @author Sean Pappalardo spappalardo@mixxx.org
  * @date Sat Apr 30 2011
  * @brief Manages creation/enumeration/deletion of hardware controllers.
  */

#include <QSet>

#include "util/trace.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/controllerlearningeventfilter.h"
#include "util/cmdlineargs.h"
#include "util/time.h"

#include "controllers/midi/portmidienumerator.h"
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
// http://developer.qt.nokia.com/wiki/Threads_Events_QObjects

// Poll every 1ms (where possible) for good controller response
#ifdef __LINUX__
// Many Linux distros ship with the system tick set to 250Hz so 1ms timer
// reportedly causes CPU hosage. See Bug #990992 rryan 6/2012
const int kPollIntervalMillis = 5;
#else
const int kPollIntervalMillis = 1;
#endif

} // anonymous namespace

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

ControllerManager::ControllerManager(UserSettingsPointer pConfig)
        : QObject(),
          m_pConfig(pConfig),
          // WARNING: Do not parent m_pControllerLearningEventFilter to
          // ControllerManager because the CM is moved to its own thread and runs
          // its own event loop.
          m_pControllerLearningEventFilter(new ControllerLearningEventFilter()),
          m_pollTimer(this),
          m_skipPoll(false) {
    qRegisterMetaType<ControllerPresetPointer>("ControllerPresetPointer");

    // Create controller mapping paths in the user's home directory.
    QString userPresets = userPresetsPath(m_pConfig);
    if (!QDir(userPresets).exists()) {
        qDebug() << "Creating user controller presets directory:" << userPresets;
        QDir().mkpath(userPresets);
    }

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

    connect(this, SIGNAL(requestInitialize()),
            this, SLOT(slotInitialize()));
    connect(this, SIGNAL(requestSetUpDevices()),
            this, SLOT(slotSetUpDevices()));
    connect(this, SIGNAL(requestShutdown()),
            this, SLOT(slotShutdown()));
    connect(this, SIGNAL(requestSave(bool)),
            this, SLOT(slotSavePresets(bool)));

    // Signal that we should run slotInitialize once our event loop has started
    // up.
    emit(requestInitialize());
}

ControllerManager::~ControllerManager() {
    emit(requestShutdown());
    m_pThread->wait();
    delete m_pThread;
    delete m_pControllerLearningEventFilter;
}

ControllerLearningEventFilter* ControllerManager::getControllerLearningEventFilter() const {
    return m_pControllerLearningEventFilter;
}

void ControllerManager::slotInitialize() {
    qDebug() << "ControllerManager:slotInitialize";

    // Initialize preset info parsers. This object is only for use in the main
    // thread. Do not touch it from within ControllerManager.
    QStringList presetSearchPaths;
    presetSearchPaths << userPresetsPath(m_pConfig)
                      << resourcePresetsPath(m_pConfig);
    m_pMainThreadPresetEnumerator = QSharedPointer<PresetInfoEnumerator>(
        new PresetInfoEnumerator(presetSearchPaths));

    // Instantiate all enumerators. Enumerators can take a long time to
    // construct since they interact with host MIDI APIs.
    m_enumerators.append(new PortMidiEnumerator());
#ifdef __HSS1394__
    m_enumerators.append(new Hss1394Enumerator());
#endif
#ifdef __BULK__
    m_enumerators.append(new BulkEnumerator());
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

void ControllerManager::slotSetUpDevices() {
    qDebug() << "ControllerManager: Setting up devices";

    updateControllerList();
    QList<Controller*> deviceList = getControllerList(false, true);

    QSet<QString> filenames;

    foreach (Controller* pController, deviceList) {
        QString name = pController->getName();

        if (pController->isOpen()) {
            pController->close();
        }

        // The filename for this device name.
        QString presetBaseName = presetFilenameFromName(name);

        // The first unique filename for this device (appends numbers at the end
        // if we have already seen a controller by this name on this run of
        // Mixxx.
        presetBaseName = firstAvailableFilename(filenames, presetBaseName);

        ControllerPresetPointer pPreset =
                ControllerPresetFileHandler::loadPreset(
                    presetBaseName + pController->presetExtension(),
                    getPresetPaths(m_pConfig));

        if (!loadPreset(pController, pPreset)) {
            // TODO(XXX) : auto load midi preset here.
            continue;
        }

        if (m_pConfig->getValueString(ConfigKey("[Controller]", presetBaseName)) != "1") {
            continue;
        }

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
        pController->applyPreset(getPresetPaths(m_pConfig), true);
    }

    maybeStartOrStopPolling();
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
    foreach (Controller* pDevice, m_controllers) {
        if (pDevice->isOpen() && pDevice->isPolling()) {
            pDevice->poll();
        }
    }

    mixxx::Duration duration = mixxx::Time::elapsed() - start;
    if (duration > mixxx::Duration::fromMillis(kPollIntervalMillis)) {
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

    // If successfully opened the device, apply the preset and save the
    // preference setting.
    if (result == 0) {
        pController->applyPreset(getPresetPaths(m_pConfig), true);

        // Update configuration to reflect controller is enabled.
        m_pConfig->setValue(ConfigKey(
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
    m_pConfig->setValue(ConfigKey(
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

void ControllerManager::slotSavePresets(bool onlyActive) {
    QList<Controller*> deviceList = getControllerList(false, true);
    QSet<QString> filenames;

    // TODO(rryan): This should be split up somehow but the filename selection
    // is dependent on all of the controllers to prevent over-writing each
    // other. We need a better solution.
    foreach (Controller* pController, deviceList) {
        if (onlyActive && !pController->isOpen()) {
            continue;
        }
        QString name = pController->getName();
        QString filename = firstAvailableFilename(
            filenames, presetFilenameFromName(name));
        QString presetPath = userPresetsPath(m_pConfig) + filename
                + pController->presetExtension();
        if (!pController->savePreset(presetPath)) {
            qWarning() << "Failed to write preset for device"
                       << name << "to" << presetPath;
        }
    }
}

// static
QList<QString> ControllerManager::getPresetPaths(UserSettingsPointer pConfig) {
    QList<QString> scriptPaths;
    scriptPaths.append(userPresetsPath(pConfig));
    scriptPaths.append(resourcePresetsPath(pConfig));
    return scriptPaths;
}

// static
bool ControllerManager::checksumFile(const QString& filename,
                                     quint16* pChecksum) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    qint64 fileSize = file.size();
    const char* pFile = reinterpret_cast<char*>(file.map(0, fileSize));

    if (pFile == NULL) {
        file.close();
        return false;
    }

    *pChecksum = qChecksum(pFile, fileSize);
    file.close();
    return true;
}

// static
QString ControllerManager::getAbsolutePath(const QString& pathOrFilename,
                                           const QStringList& paths) {
    QFileInfo fileInfo(pathOrFilename);
    if (fileInfo.isAbsolute()) {
        return pathOrFilename;
    }

    foreach (const QString& path, paths) {
        QDir pathDir(path);

        if (pathDir.exists(pathOrFilename)) {
            return pathDir.absoluteFilePath(pathOrFilename);
        }
    }

    return QString();
}

bool ControllerManager::importScript(const QString& scriptPath,
                                     QString* newScriptFileName) {
    QDir userPresets(userPresetsPath(m_pConfig));

    qDebug() << "ControllerManager::importScript importing script" << scriptPath
             << "to" << userPresets.absolutePath();

    QFile scriptFile(scriptPath);
    QFileInfo script(scriptFile);

    if (!script.exists() || !script.isReadable()) {
        qWarning() << "ControllerManager::importScript script does not exist"
                   << "or is unreadable:" << scriptPath;
        return false;
    }

    // Not fatal if we can't checksum but still warn about it.
    quint16 scriptChecksum = 0;
    bool scriptChecksumGood = checksumFile(scriptPath, &scriptChecksum);
    if (!scriptChecksumGood) {
        qWarning() << "ControllerManager::importScript could not checksum file:"
                   << scriptPath;
    }

    // The name we will save this file as in our local script mixxxdb. The
    // conflict resolution logic below will mutate this variable if the name is
    // already taken.
    QString scriptFileName = script.fileName();

    // For a file like "myfile.foo.bar.js", scriptBaseName is "myfile.foo.bar"
    // and scriptSuffix is "js".
    QString scriptBaseName = script.completeBaseName();
    QString scriptSuffix = script.suffix();
    int conflictNumber = 1;

    // This script exists.
    while (userPresets.exists(scriptFileName)) {
        // If the two files are identical. We're done.
        quint16 localScriptChecksum = 0;
        if (checksumFile(userPresets.filePath(scriptFileName), &localScriptChecksum) &&
            scriptChecksumGood && scriptChecksum == localScriptChecksum) {
            *newScriptFileName = scriptFileName;
            qDebug() << "ControllerManager::importScript" << scriptFileName
                     << "had identical checksum to a file of the same name."
                     << "Skipping import.";
            return true;
        }

        // Otherwise, we need to rename the file to a non-conflicting
        // name. Insert a .X where X is a counter that we count up until we find
        // a filename that does not exist.
        scriptFileName = QString("%1.%2.%3").arg(
            scriptBaseName,
            QString::number(conflictNumber++),
            scriptSuffix);
    }

    QString destinationPath = userPresets.filePath(scriptFileName);
    if (!scriptFile.copy(destinationPath)) {
        qDebug() << "ControllerManager::importScript could not copy script to"
                 << "local preset path:" << destinationPath;
        return false;
    }

    *newScriptFileName = scriptFileName;
    return true;
}
