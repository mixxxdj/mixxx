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
//#include "midilearnlistener.h"
//#include "midilearningprocessor.h"
//#include "midicontrolprocessor.h"
#include "midideviceportmidi.h"
#include "dlgprefmidibindings.h"
#include "mididevicemanager.h"
#include "midiledhandler.h"
#include "../mixxxcontrol.h"
#include "midimapping.h"

#define DEVICE_CONFIG_PATH BINDINGS_PATH.append("MixxxMIDIDevices")

MidiDeviceManager::MidiDeviceManager(ConfigObject<ConfigValue> * pConfig) : QObject()
{
    m_pConfig = pConfig;
    m_pDeviceSettings = new ConfigObject<ConfigValue>(DEVICE_CONFIG_PATH);
}

MidiDeviceManager::~MidiDeviceManager()
{
    closeDevices();
    MidiLedHandler::destroyHandlers();
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

    if (m_devices.empty())
        this->queryDevices();

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

void MidiDeviceManager::closeDevices()
{
    QListIterator<MidiDevice*> dev_it(m_devices);
    while (dev_it.hasNext())
    {
        qDebug() << "Closing MIDI device" << dev_it.peekNext()->getName();
        dev_it.next()->close();
    }
}

/** Enumerate the MIDI devices 
  * This method needs a bit of intelligence because PortMidi (and the underlying MIDI APIs) like to split
  * output and input into separate devices. Eg. PortMidi would tell us the Hercules is two half-duplex devices.
  * To help simplify a lot of code, we're going to aggregate these two streams into a single full-duplex device.
  */
void MidiDeviceManager::queryDevices()
{
    qDebug() << "Scanning MIDI devices:";
    int iNumDevices = Pm_CountDevices();
    
    QListIterator<MidiDevice*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }
    
    m_devices.clear();
    
    const PmDeviceInfo *deviceInfo, *inputDeviceInfo, *outputDeviceInfo;
    int inputDevIndex, outputDevIndex;
    QMap<int,QString> unassignedOutputDevices;
    
    // Build a complete list of output devices for later pairing
    for (int i = 0; i < iNumDevices; i++)
    {
        deviceInfo = Pm_GetDeviceInfo(i);
        if (deviceInfo->output) {
            qDebug() << " Found output device" << "#" << i << deviceInfo->name;
            QString deviceName = deviceInfo->name;
            unassignedOutputDevices[i] = deviceName;
        }
    }

    // Search for input devices and pair them with output devices if applicable
    for (int i = 0; i < iNumDevices; i++)
    {
        deviceInfo = Pm_GetDeviceInfo(i);
        
        //If we found an input device
        if (deviceInfo->input)
        {
            qDebug() << " Found input device" << "#" << i << deviceInfo->name;
            inputDeviceInfo = deviceInfo;
            inputDevIndex = i;
            
            //Reset our output device variables before we look for one incase we find none.
            outputDeviceInfo = NULL;
            outputDevIndex = -1;
            
            //Search for a corresponding output device
            QMapIterator<int, QString> j(unassignedOutputDevices);
            while (j.hasNext()) {
                j.next();
                
                QString deviceName = inputDeviceInfo->name;
                QString outputName = QString(j.value());

                //Some device drivers prepend "To" and "From" to the names
                //of their MIDI ports. If the output and input device names
                //don't match, let's try trimming those words from the start,
                //and seeing if they then match.
                if (outputName != deviceName) {
                    // Ignore "From" text in the device names
                    if (deviceName.indexOf("from",0,Qt::CaseInsensitive)!=-1) 
                        deviceName = deviceName.right(deviceName.length()-4);
                    // Ignore "To" text in the device names
                    if (outputName.indexOf("to",0,Qt::CaseInsensitive)!=-1) 
                        outputName = outputName.right(outputName.length()-2);
                }

                if (outputName == deviceName) {
                    outputDevIndex = j.key();
                    outputDeviceInfo = Pm_GetDeviceInfo(outputDevIndex);
                    
                    unassignedOutputDevices.remove(outputDevIndex);
                    
                    qDebug() << "    Linking to output device #" << outputDevIndex << outputName;
                    break;
                }
            }

            //So at this point in the code, we either have an input-only MIDI device (outputDeviceInfo == NULL)
            //or we've found a matching output MIDI device (outputDeviceInfo != NULL).
            
            //.... so create our (aggregate) MIDI device!            
            MidiDevicePortMidi *currentDevice = new MidiDevicePortMidi(/*new MidiControlProcessor(NULL)*/ NULL,
                                                                          inputDeviceInfo,
                                                                          outputDeviceInfo,
                                                                          inputDevIndex,
                                                                          outputDevIndex); 
            m_devices.push_back((MidiDevice*)currentDevice);
            
        }

//         if (deviceInfo->input || deviceInfo->output)
//         {
//             MidiDevicePortMidi *currentDevice = new MidiDevicePortMidi(new MidiControlProcessor(NULL), deviceInfo, i);
//             m_devices.push_back((MidiDevice*)currentDevice);
//         }
    }
}

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
