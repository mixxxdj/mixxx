/**
  * @file mididevicemanager.cpp
  * @author Albert Santoni alberts@mixxx.org & Sean Pappalardo pegasus@c64.org
  * @date Thu Dec 18 2008
  * @brief Manages creation/enumeration/deletion of MidiDevices.
  */

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include "mididevice.h"
#include "dlgprefmidibindings.h"
#include "mididevicemanager.h"
#include "midiledhandler.h"
#include "../mixxxcontrol.h"
#include "midimapping.h"

#define DEVICE_CONFIG_PATH BINDINGS_PATH.append("MixxxMIDIDevices")

MidiDeviceManager::MidiDeviceManager(ConfigObject<ConfigValue> * pConfig) : QObject()
{
    m_pConfig = pConfig;

    m_pPMEnumerator = new PortMidiEnumerator();
#ifdef __HSS1394__
    m_pHSSEnumerator = new Hss1394Enumerator();
#endif
}

MidiDeviceManager::~MidiDeviceManager()
{
    //Delete enumerators and they'll delete their Devices
    delete m_pPMEnumerator;
    MidiLedHandler::destroyHandlers();
#ifdef __HSS1394__
    delete m_pHSSEnumerator;
#endif
}

void MidiDeviceManager::saveMappings(bool onlyActive) {
    // Write out MIDI mappings for currently connected devices
    QList<MidiDevice*> deviceList = getDeviceList(false, true);
    QListIterator<MidiDevice*> it(deviceList);

    QList<QString> filenames;

    while (it.hasNext())
    {
        MidiDevice *cur= it.next();
        if (onlyActive && !cur->isOpen()) continue;
        MidiMapping *mapping = cur->getMidiMapping();
        QString name = cur->getName();

        QString ofilename = name.right(name.size()-name.indexOf(" ")-1).replace(" ", "_");

        QString filename = ofilename;

        int i=1;
        while (filenames.contains(filename)) {
            i++;
            filename = QString("%1--%2").arg(ofilename).arg(i);
        }

        filenames.append(filename);
        mapping->savePreset(BINDINGS_PATH.append(filename + MIDI_MAPPING_EXTENSION));
    }
}

QList<MidiDevice*> MidiDeviceManager::getDeviceList(bool bOutputDevices, bool bInputDevices)
{
    qDebug() << "MidiDeviceManager::getDeviceList";
    bool bMatchedCriteria = false;   //Whether or not the current device matched the filtering criteria

    if (m_devices.empty()) {
        m_devices = m_pPMEnumerator->queryDevices();
#ifdef __HSS1394__
        m_devices.append(m_pHSSEnumerator->queryDevices());
#endif
    }

    //Create a list of MIDI devices filtered to match the given input/output options.
    QList<MidiDevice*> filteredDeviceList;
    QListIterator<MidiDevice*> dev_it(m_devices);
    while (dev_it.hasNext())
    {
        bMatchedCriteria = false;                //Reset this for the next device.
        MidiDevice *device = dev_it.next();

        if ((bOutputDevices == device->isOutputDevice()) ||
            (bInputDevices == device->isInputDevice())) {
            bMatchedCriteria = true;
        }

        if (bMatchedCriteria)
            filteredDeviceList.push_back(device);
    }
    return filteredDeviceList;
}

//void MidiDeviceManager::closeDevices()
//{
//    QListIterator<MidiDevice*> dev_it(m_devices);
//    while (dev_it.hasNext())
//    {
//        qDebug() << "Closing MIDI device" << dev_it.peekNext()->getName();
//        dev_it.next()->close();
//    }
//}

/** Open whatever MIDI devices are selected in the preferences. */
int MidiDeviceManager::setupDevices()
{
    QList<MidiDevice*> deviceList = getDeviceList(false, true);
    QListIterator<MidiDevice*> it(deviceList);

    qDebug() << "MidiDeviceManager: Setting up devices";

    QList<QString> filenames;

    while (it.hasNext())
    {
        MidiDevice *cur= it.next();
        MidiMapping *mapping = cur->getMidiMapping();
        QString name = cur->getName();
        mapping->setName(name);

        cur->close();

        QString ofilename = name.right(name.size()-name.indexOf(" ")-1).replace(" ", "_");

        QString filename = ofilename;

        int i=1;
        while (filenames.contains(filename)) {
            i++;
            filename = QString("%1--%2").arg(ofilename).arg(i);
        }

        filenames.append(filename);
        mapping->loadPreset(BINDINGS_PATH.append(filename + MIDI_MAPPING_EXTENSION),true);

        if ( m_pConfig->getValueString(ConfigKey("[Midi]", name.replace(" ", "_"))) != "1" )
            continue;

        qDebug() << "Opening Device:" << name;

        cur->open();
        // Prevents a deadlock if the device is sending data when it's initialized
        cur->setReceiveInhibit(true);
        mapping->applyPreset();
        cur->setReceiveInhibit(false);
    }

    return 0;
}


/** This should be in MidiProcessor because it's bindings related */
QStringList MidiDeviceManager::getConfigList(QString path)
{
    // Make sure list is empty
    QStringList configs;
    configs.clear();

    // Get list of available midi configurations
    QDir dir(path);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << "*.midi.xml" << "*.MIDI.XML");

    //const QFileInfoList *list = dir.entryInfoList();
    //if (dir.entryInfoList().empty())
    {
        QListIterator<QFileInfo> it(dir.entryInfoList());
        QFileInfo fi;                          // pointer for traversing
        while (it.hasNext())
        {
            fi = it.next();
            configs.append(fi.fileName());
        }
    }

    return configs;
}


void MidiDeviceManager::associateInputAndOutputDevices(MidiDevice* inputDevice, QString outputDeviceName)
{
    //TODO: This function needs to be updated to work with our "aggregate" input/ouput MidiDevice class
    //      or just simply removed all together. I just sent out a mixxx-devel email with more history
    //      on this, check the archive if you need more info. -- Albert Nov 9/09 (1.8 CRUNCH TIME!)
    //

/*
    //Find the output MidiDevice object that corresponds to outputDeviceName.
    QListIterator<MidiDevice*> dev_it(m_devices);
    MidiDevice* outputDevice = NULL;
    while (dev_it.hasNext()) {
        outputDevice = dev_it.next();
        if (outputDevice->getName() == outputDeviceName) {
            qDebug() << "associating input dev" << inputDevice->getName() << "with" << outputDeviceName;
            break;
        }
    }

    if (outputDevice == NULL) //No output device matched outputDeviceName...
        return;

    //Tell the input device that it's corresponding output device is... outputDevice.
    inputDevice->setOutputDevice(outputDevice);
    */
}
