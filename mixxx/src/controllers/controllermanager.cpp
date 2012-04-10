/**
  * @file controllermanager.cpp
  * @author Sean Pappalardo spappalardo@mixxx.org
  * @date Sat Apr 30 2011
  * @brief Manages creation/enumeration/deletion of hardware controllers.
  */

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QSet>

#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"

#include "controllers/midi/portmidienumerator.h"
#ifdef __HSS1394__
    #include "controllers/midi/hss1394enumerator.h"
#endif

#ifdef __HID__
    #include "controllers/hidenumerator.h"
#endif
#ifdef __OSC__
    #include "controllers/osc/oscenumerator.h"
#endif

// http://developer.qt.nokia.com/wiki/Threads_Events_QObjects

ControllerProcessor::ControllerProcessor(ControllerManager* pManager)
        : QObject(pManager),
          m_pManager(pManager),
          m_pollingTimerId(0) {
}

ControllerProcessor::~ControllerProcessor() {
}

void ControllerProcessor::startPolling() {
    // Set up polling timer

    // This makes use of every QObject's internal timer mechanism. Nice, clean, and simple.
    // See http://doc.trolltech.com/4.6/qobject.html#startTimer for details

    // Poll every 1ms (where possible) for good controller response
    if (m_pollingTimerId == 0) {
        qDebug() << "Starting controller polling";
        m_pollingTimerId = startTimer(1);
        if (m_pollingTimerId == 0) qWarning() << "Could not start polling timer!";
    }
    //else qWarning() << "Polling timer already running!";
}

void ControllerProcessor::stopPolling() {
    if (m_pollingTimerId == 0) {
        return;
    }

    // Stop the timer
    killTimer(m_pollingTimerId);
    m_pollingTimerId = 0;
    qDebug() << "Controller polling stopped";
}

void ControllerProcessor::timerEvent(QTimerEvent *event) {
    bool poll = false;
    // See if this is the polling timer
    if (event->timerId() == m_pollingTimerId) {
        poll = true;
    }

    // Pass it on to the active controllers in any case
    foreach (Controller* pDevice, m_pManager->getControllers()) {
        if (pDevice->isOpen()) {
            pDevice->timerEvent(event, poll);
        }
    }
}

ControllerManager::ControllerManager(ConfigObject<ConfigValue> * pConfig) :
        QObject(),
        m_pConfig(pConfig) {

    // Instantiate all enumerators
    m_enumerators.append(new PortMidiEnumerator());
#ifdef __HSS1394__
    m_enumerators.append(new Hss1394Enumerator());
#endif
#ifdef __HID__
    m_enumerators.append(new HidEnumerator());
#endif
#ifdef __OSC__
    m_enumerators.append(new OscEnumerator());
#endif

    m_pThread = new QThread;
    m_pThread->setObjectName("Controller");

    m_pProcessor = new ControllerProcessor(this);

    this->moveToThread(m_pThread); // implies m_pProcessor->moveToThread(m_pThread);

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
    delete m_pProcessor;
    delete m_pThread;
}

void ControllerManager::slotShutdown() {
    m_pProcessor->stopPolling();

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

        QScopedPointer<ControllerPresetFileHandler> handler(pController->getFileHandler());
        if (!handler) {
            qDebug() << "Failed to get a handler for the controller.";
            continue;
        }

        QString presetPath = PRESETS_PATH.append(filename + pController->presetExtension());
        ControllerPreset* preset = handler->load(presetPath, name, true);
        if (preset == NULL) {
            qDebug() << "Couldn't load preset" << presetPath;
            continue;
        }
        pController->setPreset(*preset);

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
        if (pController->needPolling()) {
            polling = true;
        }
    }

    // Start polling of applicable controller APIs
    enablePolling(polling);

    return error;
}

void ControllerManager::enablePolling(bool enable) {
    if (enable) {
        m_pProcessor->startPolling();
    } else {
        // This controller doesn't need it, but check to make sure others don't
        // before disabling it
        foreach (Controller* pController, m_controllers) {
            // (re-using enable here)
            if (pController->isOpen() && pController->needPolling()) {
                enable = true;
            }
        }
        if (!enable) {
            m_pProcessor->stopPolling();
        }
    }
}

QList<QString> ControllerManager::getPresetList(QString extension) {
    // Paths to search for controller presets
    QList<QString> controllerDirPaths;
    controllerDirPaths.append(LPRESETS_PATH);
    controllerDirPaths.append(m_pConfig->getConfigPath().append("controllers/"));

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
        QString presetPath = PRESETS_PATH.append(filename + pController->presetExtension());
        if (!pController->savePreset(presetPath)) {
            qDebug() << "Failed to write preset for device"
                     << name << "to" << presetPath;
        }
    }
}
