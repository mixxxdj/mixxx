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
#include "../mixxxcontrol.h"
#include "midimapping.h"

#define DEVICE_CONFIG_PATH BINDINGS_PATH.append("MixxxMIDIDevices")

MidiDeviceManager::MidiDeviceManager(ConfigObject<ConfigValue> * pConfig) : QObject()
{
    m_pConfig = pConfig;
    m_pPrimaryMidiDevice = NULL;
    m_pDeviceSettings = new ConfigObject<ConfigValue>(DEVICE_CONFIG_PATH);
}

MidiDeviceManager::~MidiDeviceManager()
{
    closeDevices();
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
        
        // TODO: Make this work with up to 99 devices
        QString ofilename = name.right(name.size()-3).replace(" ", "_");
        
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
            // Ignore "To" text in the device names
            if (deviceName.indexOf("to",0,Qt::CaseInsensitive)!=-1) deviceName = deviceName.right(deviceName.length()-2);
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
                
                // Ignore "From" text in the device names
                QString deviceName = inputDeviceInfo->name;
                if (deviceName.indexOf("from",0,Qt::CaseInsensitive)!=-1) deviceName = deviceName.right(deviceName.length()-4);

                QByteArray outputName = QString(j.value()).toUtf8();
                if (strcmp(outputName, deviceName) == 0) {
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
        
        // TODO: Make this work with up to 99 devices
        QString ofilename = name.right(name.size()-3).replace(" ", "_");
        
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
        mapping->applyPreset();
        
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

/*
void MidiDeviceManager::enableMidiLearn(DlgPrefMidiBindings *listener)
{
    MidiLearningProcessor* proc = new MidiLearningProcessor();
    //Save the old MIDI processor so we can restore it when MIDI learn is disabled again.
    m_pTempControlProc = m_pPrimaryMidiDevice->getMidiProcessor(); 
    m_pPrimaryMidiDevice->setMidiProcessor(proc); //MidiDevice will delete proc when it's done, so no memory leak. 
    
    connect(proc, SIGNAL(midiEvent(ConfigValueMidi *, QString)), listener, SLOT(singleLearn(ConfigValueMidi *, QString)));
    connect(proc, SIGNAL(midiEvent(ConfigValueMidi *, QString)), listener, SLOT(groupLearn(ConfigValueMidi *, QString)));
    
}

void MidiDeviceManager::disableMidiLearn()
{
    //If there's no primary MIDI device, just return for safety.
    if (!m_pPrimaryMidiDevice)
        return;
    
    //Get the midi processor from the primary midi device, and see if it's a MidiLearningProcessor...
    MidiLearningProcessor* learn = dynamic_cast<MidiLearningProcessor*>(m_pPrimaryMidiDevice->getMidiProcessor());
    //If the midi processor wasn't a MidiLearningProcessor, then we don't want to do anything. Eg. this happens
    //if disableMidiLearn() gets called when MIDI learn is already disabled.
    if (!learn)
        return;
        
    delete learn;
    //Restore the old MIDI processor (re-enables the MIDI mapping).
    m_pPrimaryMidiDevice->setMidiProcessor(m_pTempControlProc);
    m_pTempControlProc = NULL;
}
*/

void MidiDeviceManager::associateInputAndOutputDevices(MidiDevice* inputDevice, QString outputDeviceName)
{
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
}
