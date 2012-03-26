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

// #include <QtCore>
#include "controllermanager.h"
#include "defs_controllers.h"

// http://developer.qt.nokia.com/wiki/Threads_Events_QObjects

ControllerProcessor::ControllerProcessor(ControllerManager * pManager) : QObject(pManager) {
    m_pManager = pManager;
    m_pollingTimerId = 0;
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
//     else qWarning() << "Polling timer already running!";
}

void ControllerProcessor::stopPolling() {
    if (m_pollingTimerId == 0) return;

    // Stop the timer
    killTimer(m_pollingTimerId);
    m_pollingTimerId = 0;
    qDebug() << "Controller polling stopped";
}

void ControllerProcessor::timerEvent(QTimerEvent *event) {
    bool poll = false;
    // See if this is the polling timer
    if (event->timerId() == m_pollingTimerId) poll=true;

    // Pass it on to the active controllers in any case
    QListIterator<Controller*> dev_it(m_pManager->getControllers());
    while (dev_it.hasNext())
    {
        Controller *device = dev_it.next();
        if (device->isOpen()) device->timerEvent(event,poll);
    }
}

// ------ ControllerManager ------

ControllerManager::ControllerManager(ConfigObject<ConfigValue> * pConfig) : QObject() {
    m_pConfig = pConfig;
//     m_pDeviceSettings = new ConfigObject<ConfigValue>(DEVICE_CONFIG_PATH);

    // Instantiate all enumerators
    m_pPMEnumerator = new PortMidiEnumerator();
#ifdef __HSS1394__
    m_pHSSEnumerator = new Hss1394Enumerator();
#endif
#ifdef __HID__
    m_pHIDEnumerator = new HidEnumerator();
#endif
#ifdef __OSC__
    m_pOSCEnumerator = new OscEnumerator();
#endif

    m_pThread = new QThread;
    m_pThread->setObjectName("Controller");

    m_pProcessor = new ControllerProcessor(this);

    this->moveToThread(m_pThread); // implies m_pProcessor->moveToThread(m_pThread);
    
    // Controller processing needs to be prioritized since it can affect the audio
    //  directly, like when scratching
    m_pThread->start(QThread::HighPriority);
    
    connect(this, SIGNAL(requestSetUpDevices()), this, SLOT(slotSetUpDevices()));
    connect(this, SIGNAL(requestShutdown()), this, SLOT(slotShutdown()));
    connect(this, SIGNAL(requestSave(bool)), this, SLOT(slotSavePresets(bool)));
}

ControllerManager::~ControllerManager() {
    m_pThread->wait();
    delete m_pProcessor;
    delete m_pThread;
}

void ControllerManager::slotShutdown() {
    m_pProcessor->stopPolling();
    
    //Delete enumerators and they'll delete their Devices
    delete m_pPMEnumerator;
#ifdef __HSS1394__
    delete m_pHSSEnumerator;
#endif
#ifdef __HID__
    delete m_pHIDEnumerator;
#endif
#ifdef __OSC__
    delete m_pOSCEnumerator;
#endif
    // Stop the processor after the enumerators since the engines live in it
    m_pThread->quit();
}

void ControllerManager::updateControllerList() {
    
    QList<Controller*> newDeviceList;
    newDeviceList.append(m_pPMEnumerator->queryDevices());
#ifdef __HSS1394__
    newDeviceList.append(m_pHSSEnumerator->queryDevices());
#endif
#ifdef __HID__
    newDeviceList.append(m_pHIDEnumerator->queryDevices());
#endif
#ifdef __OSC__
    newDeviceList.append(m_pOSCEnumerator->queryDevices());
#endif

    m_mControllerList.lock();
    if (newDeviceList != m_controllers) {
        m_controllers = newDeviceList;
        m_mControllerList.unlock();
        emit(devicesChanged());
    }
    else m_mControllerList.unlock();
}

QList<Controller*> ControllerManager::getControllerList(bool bOutputDevices, bool bInputDevices) {
    qDebug() << "ControllerManager::getControllerList";
    
    bool bMatchedCriteria = false;   //Whether or not the current device matched the filtering criteria

    //Create a list of controllers filtered to match the given input/output options.
    QList<Controller*> filteredDeviceList;
    
    m_mControllerList.lock();
    QList<Controller*> controllers = m_controllers;
    m_mControllerList.unlock();
    
    QListIterator<Controller*> dev_it(controllers);
    while (dev_it.hasNext())
    {
        bMatchedCriteria = false;                //Reset this for the next device.
        Controller *device = dev_it.next();

        if ((bOutputDevices == device->isOutputDevice()) ||
            (bInputDevices == device->isInputDevice())) {
            bMatchedCriteria = true;
        }

        if (bMatchedCriteria)
            filteredDeviceList.push_back(device);
    }
    return filteredDeviceList;
}

/** Open whatever controllers are selected in the preferences. */
// This currently only runs on start-up but maybe should instead be signaled by
//  the preferences dialog on apply, and only open/close changed devices
int ControllerManager::slotSetUpDevices() {
    qDebug() << "ControllerManager: Setting up devices";

    updateControllerList();
    QList<Controller*> deviceList = getControllerList(false, true);
    QListIterator<Controller*> it(deviceList);

    QList<QString> filenames;
    int error = 0;

    bool polling = false;

    while (it.hasNext())
    {
        Controller *cur= it.next();
        QString name = cur->getName();

        if (cur->isOpen()) cur->close();

        QString ofilename = name.replace(" ", "_");

        QString filename = ofilename;

        int i=1;
        while (filenames.contains(filename)) {
            i++;
            filename = QString("%1--%2").arg(ofilename, QString::number(i));
        }

        filenames.append(filename);
        m_pConfig->getValueString(ConfigKey("[ControllerPreset]", name.replace(" ", "_")));
//         qDebug() << "ControllerPreset" << m_pConfig->getValueString(ConfigKey("[ControllerPreset]", name.replace(" ", "_")));
        cur->loadPreset(PRESETS_PATH.append(filename + cur->presetExtension()),true);

        if ( m_pConfig->getValueString(ConfigKey("[Controller]", name.replace(" ", "_"))) != "1" )
            continue;

        qDebug() << "Opening controller:" << name;
        
        int value = cur->open();
        if (value != 0) {
            qWarning() << "  There was a problem opening" << name;
            if (error==0) error=value;
            continue;
        }
        cur->applyPreset();

        // Only enable polling when open controllers actually need it
        if (!polling && cur->needPolling()) polling = true;
    }

    // Start polling of applicable controller APIs
    enablePolling(polling);
    
    return error;
}

void ControllerManager::enablePolling(bool enable) {
    if (enable) m_pProcessor->startPolling();
    else {
        // This controller doesn't need it, but check to make sure others don't
        //  before disabling it
        QListIterator<Controller*> it(m_controllers);
        while (it.hasNext()) {
            Controller *cur = it.next();
            // (re-using enable here)
            if (cur->isOpen() && cur->needPolling()) enable = true;
        }
        if (!enable) m_pProcessor->stopPolling();
    }
}

QList<QString> ControllerManager::getPresetList(QString extension)
{
    QList<QString> presets;
    // Make sure list is empty
    presets.clear();
    
    // Paths to search for controller presets
    QList<QString> controllerDirPaths;
    controllerDirPaths.append(LPRESETS_PATH);
    controllerDirPaths.append(m_pConfig->getConfigPath().append("controllers/"));
    
    QListIterator<QString> itpth(controllerDirPaths);
    while (itpth.hasNext()) {
        QDirIterator it(itpth.next());
        while (it.hasNext())
        {
            it.next(); //Advance iterator. We get the filename from the next line. (It's a bit weird.)
            QString curMapping = it.fileName();
            if (curMapping.endsWith(extension))
            {
                curMapping.chop(QString(extension).length()); //chop off the extension
                presets.append(curMapping);
            }
        }
    }
    
    return presets;
}

void ControllerManager::slotSavePresets(bool onlyActive) {
    
    QList<Controller*> deviceList = getControllerList(false, true);
    QListIterator<Controller*> it(deviceList);
    
    QList<QString> filenames;
    
    while (it.hasNext())
    {
        Controller *cur= it.next();
        if (onlyActive && !cur->isOpen()) continue;
        QString name = cur->getName();
        
        QString ofilename = name.replace(" ", "_");
        
        QString filename = ofilename;
        
        int i=1;
        while (filenames.contains(filename)) {
            i++;
            filename = QString("%1--%2").arg(ofilename, QString::number(i));
        }
        
        filenames.append(filename);
        cur->savePreset(PRESETS_PATH.append(filename + cur->presetExtension()));
    }
}