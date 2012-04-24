/**
  * @file controllermanager.cpp
  * @author Sean Pappalardo spappalardo@mixxx.org
  * @date Sat Apr 30 2011
  * @brief Manages creation/enumeration/deletion of hardware controllers.
  */

#include <QSet>

#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"

#include "controllers/midi/portmidienumerator.h"
#ifdef __HSS1394__
    #include "controllers/midi/hss1394enumerator.h"
#endif

#ifdef __HID__
    #include "controllers/hid/hidenumerator.h"
#endif

// http://developer.qt.nokia.com/wiki/Threads_Events_QObjects

// Poll every 1ms (where possible) for good controller response
const int kPollIntervalMillis = 1;

ControllerManager::ControllerManager(ConfigObject<ConfigValue> * pConfig) :
        QObject(),
        m_pConfig(pConfig),
        m_pollTimer(this) {

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
}

void ControllerManager::slotShutdown() {
    stopPolling();

    // Clear m_enumerators before deleting the enumerators to prevent other code
    // paths from accessing them.
    QList<ControllerEnumerator*> enumerators = m_enumerators;
    m_enumerators.clear();

    // Delete enumerators and they'll delete their Devices
    foreach (ControllerEnumerator* pEnumerator, enumerators) {
        delete pEnumerator;
    }

    // Stop the processor after the enumerators since the engines live in it
    m_pThread->quit();
}

void ControllerManager::updateControllerList() {
    if (m_enumerators.isEmpty()) {
        qWarning() << "updateControllerList called but no enumerators have been added!";
        return;
    }

    QList<Controller*> newDeviceList;
    foreach (ControllerEnumerator* pEnumerator, m_enumerators) {
        newDeviceList.append(pEnumerator->queryDevices());
    }

    QMutexLocker locker(&m_mControllerList);
    if (newDeviceList != m_controllers) {
        m_controllers = newDeviceList;
        locker.unlock();
        emit(devicesChanged());
    }
}

QList<Controller*> ControllerManager::getControllerList(bool bOutputDevices, bool bInputDevices) {
    qDebug() << "ControllerManager::getControllerList";

    m_mControllerList.lock();
    QList<Controller*> controllers = m_controllers;
    m_mControllerList.unlock();

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
    bool polling = false;

    foreach (Controller* pController, deviceList) {
        QString name = pController->getName();

        if (pController->isOpen()) {
            pController->close();
        }

        const QString ofilename = name.replace(" ", "_");
        QString filename = ofilename;
        int i=1;
        while (filenames.contains(filename)) {
            i++;
            filename = QString("%1--%2").arg(ofilename, QString::number(i));
        }
        filenames.insert(filename);

        if (!loadPreset(pController, name, true)) continue;

        //qDebug() << "ControllerPreset" << m_pConfig->getValueString(ConfigKey("[ControllerPreset]", ofilename));

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
        pController->applyPreset();

        // Only enable polling when open controllers actually need it
        if (pController->isPolling()) {
            polling = true;
        }
    }

    // Start polling of applicable controller APIs
    enablePolling(polling);

    return error;
}

void ControllerManager::enablePolling(bool enable) {
    if (enable) {
        startPolling();
    } else {
        // This controller doesn't need it, but check to make sure others don't
        // before disabling it
        foreach (Controller* pController, m_controllers) {
            // (re-using enable here)
            if (pController->isOpen() && pController->isPolling()) {
                enable = true;
            }
        }
        if (!enable) {
            stopPolling();
        }
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
    foreach (Controller* pDevice, m_controllers) {
        if (pDevice->isOpen() && pDevice->isPolling()) {
            pDevice->poll();
        }
    }
}

QList<QString> ControllerManager::getPresetList(QString extension) {
    // Paths to search for controller presets
    QList<QString> controllerDirPaths;
    QString configPath = m_pConfig->getConfigPath();
//     controllerDirPaths.append(configPath.append("presets/"));
//     controllerDirPaths.append(configPath.append("midi/"));
    controllerDirPaths.append(configPath.append("controllers/"));

    // Should we also show the presets from USER_PRESETS_PATH?
    //  That could be confusing.

    QList<QString> presets;
    foreach (QString dirPath, controllerDirPaths) {
        QDirIterator it(dirPath);
        while (it.hasNext()) {
            // Advance iterator. We get the filename from the next line. (It's a
            // bit weird.)
            it.next();

            QString curPreset = it.fileName();
            if (curPreset.endsWith(extension)) {
                curPreset.chop(QString(extension).length()); //chop off the extension
                presets.append(curPreset);
            }
        }
    }
    return presets;
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

    QString filenameWithExt = filename + pController->presetExtension();
    QString filepath = USER_PRESETS_PATH + filenameWithExt;

    // If the file isn't present in the user's directory, check res/
    if (!QFile::exists(filepath)) filepath = m_pConfig->getConfigPath()
                                    .append("controllers/") + filenameWithExt;

    if (QFile::exists(filepath)) {
        ControllerPreset* preset = handler->load(filepath, filename,
                                                 force);
        if (preset == NULL) {
            qWarning() << "Unable to load preset" << filepath;
        }
        else {
            pController->setPreset(*preset);
            // Save the file path/name in the config so it can be auto-loaded at startup next time
            m_pConfig->set(ConfigKey("[ControllerPreset]",
                                     pController->getName().replace(" ", "_")),
                           filepath);
//             qDebug() << "Successfully loaded preset" << filepath;
            return true;
        }
    }
    qWarning() << "Cannot find" << filenameWithExt << "in either res/"
             << "or the user's Mixxx directory (~/.mixxx/controllers/)";
    return false;
}

void ControllerManager::slotSavePresets(bool onlyActive) {
    QList<Controller*> deviceList = getControllerList(false, true);
    QSet<QString> filenames;

    foreach (Controller* pController, deviceList) {
        if (onlyActive && !pController->isOpen()) {
            continue;
        }
        QString name = pController->getName();
        QString ofilename = name.replace(" ", "_");
        QString filename = ofilename;
        int i=1;
        while (filenames.contains(filename)) {
            i++;
            filename = QString("%1--%2").arg(ofilename, QString::number(i));
        }
        filenames.insert(filename);
        QString presetPath = USER_PRESETS_PATH + filename
                                + pController->presetExtension();
        if (!pController->savePreset(presetPath)) {
            qWarning() << "Failed to write preset for device"
                     << name << "to" << presetPath;
        }
    }
}
