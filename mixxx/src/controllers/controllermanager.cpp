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

#define LPRESETS_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("presets/")
#define CONTROLLER_PRESET_EXTENSION ".cntrlr.xml"
#define MIDI_MAPPING_EXTENSION ".midi.xml"
#define DEFAULT_DEVICE_PRESET BINDINGS_PATH.append(m_deviceName.replace(" ", "_") + CONTROLLER_PRESET_EXTENSION)
#define DEFAULT_MIDI_DEVICE_PRESET BINDINGS_PATH.append(m_deviceName.right(m_deviceName.size()-m_deviceName.indexOf(" ")-1).replace(" ", "_") + MIDI_MAPPING_EXTENSION)

ControllerManager::ControllerManager(ConfigObject<ConfigValue> * pConfig) : QObject()
{
    m_pConfig = pConfig;
//     m_pDeviceSettings = new ConfigObject<ConfigValue>(DEVICE_CONFIG_PATH);

#ifdef __HID__
    m_pHIDEnumerator = new HidEnumerator();
#endif
#ifdef __OSC__
    m_pOSCEnumerator = new OscEnumerator();
#endif
}

ControllerManager::~ControllerManager()
{
    //Delete enumerators and they'll delete their Devices
#ifdef __HID__
    delete m_pHIDEnumerator;
#endif
#ifdef __OSC__
    delete m_pOSCEnumerator;
#endif
}

QList<Controller*> ControllerManager::getControllerList(bool bOutputDevices, bool bInputDevices)
{
    qDebug() << "ControllerManager::getControllerList";
    bool bMatchedCriteria = false;   //Whether or not the current device matched the filtering criteria

    if (m_controllers.empty()) {
#ifdef __HID__
        m_controllers.append(m_pHIDEnumerator->queryDevices());
#endif
    }

    //Create a list of controllers filtered to match the given input/output options.
    QList<Controller*> filteredDeviceList;
    QListIterator<Controller*> dev_it(m_controllers);
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
int ControllerManager::setupDevices()
{
    QList<Controller*> deviceList = getControllerList(false, true);
    QListIterator<Controller*> it(deviceList);

    qDebug() << "ControllerManager: Setting up devices";

    QList<QString> filenames;
    int error = 0;

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
            filename = QString("%1--%2").arg(ofilename).arg(i);
        }

        filenames.append(filename);
//         mapping->loadPreset(BINDINGS_PATH.append(filename + MIDI_MAPPING_EXTENSION),true);

        if ( m_pConfig->getValueString(ConfigKey("[Controller]", name.replace(" ", "_"))) != "1" )
            continue;

        qDebug() << "Opening controller:" << name;
        
        int value = cur->open();
        if (value != 0) {
            qWarning() << "  There was a problem opening" << name;
            if (error==0) error=value;
            continue;
        }
//         mapping->applyPreset();
    }
    return error;
}

QList<QString> ControllerManager::getPresetList(bool midi)
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
            if(midi) {
                if (curMapping.endsWith(MIDI_MAPPING_EXTENSION)) //blah, thanks for nothing Qt
                {
                    curMapping.chop(QString(MIDI_MAPPING_EXTENSION).length()); //chop off the extension
                    presets.append(curMapping);
                }
            }
            else {
                if (curMapping.endsWith(CONTROLLER_PRESET_EXTENSION)) //blah, thanks for nothing Qt
                {
                    curMapping.chop(QString(CONTROLLER_PRESET_EXTENSION).length()); //chop off the extension
                    presets.append(curMapping);
                }
            }
        }
    }
    
    return presets;
}
