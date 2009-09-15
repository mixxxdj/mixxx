/**
  * @file mididevicemanager.cpp
  * @author Albert Santoni alberts@mixxx.org
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

#define DEVICE_CONFIG_PATH QDir::homePath().append("/").append(SETTINGS_PATH).append("MixxxMIDIDevices")

MidiDeviceManager::MidiDeviceManager(ConfigObject<ConfigValue> * pConfig) : QObject()
{
    m_pConfig = pConfig;
    m_pPrimaryMidiDevice = NULL;
    m_pDeviceSettings = new ConfigObject<ConfigValue>(DEVICE_CONFIG_PATH);
}

MidiDeviceManager::~MidiDeviceManager()
{
    QList<MidiDevice*> deviceList = getDeviceList(false, true);
    QListIterator<MidiDevice*> it(deviceList);
    
    
    while (it.hasNext())
    {
        MidiDevice *cur= it.next();
        MidiMapping *mapping = cur->getMidiMapping();
        QString name = cur->getName();
        
        mapping->savePreset(
                QDir::homePath().append("/").append(SETTINGS_PATH).
                append(name.right(name.size()-3).replace(" ", "_") + ".midi.xml")
        );
    }
    
    closeDevices();
}

QList<MidiDevice*> MidiDeviceManager::getDeviceList(bool bOutputDevices, bool bInputDevices)
{
    qDebug() << "MidiDeviceManager::getDeviceList";
    bool bMatchedCriteria = true;   //Whether or not the current device matched the filtering criteria

    if (m_devices.empty())
        this->queryDevices();

    //Create a list of MIDI devices filtered to match the given input/output options.
    QList<MidiDevice*> filteredDeviceList;
    QListIterator<MidiDevice*> dev_it(m_devices);
    while (dev_it.hasNext())
    {
        bMatchedCriteria = true;                //Reset this for the next device.
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
    int iNumDevices = Pm_CountDevices();
    
    QListIterator<MidiDevice*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }
    
    m_devices.clear();
    
    const PmDeviceInfo *deviceInfo, *inputDeviceInfo, *outputDeviceInfo;
    int inputDevIndex, outputDevIndex;
    for (int i = 0; i < iNumDevices; i++)
    {
        deviceInfo = Pm_GetDeviceInfo(i);
        
        qDebug() << deviceInfo->name << deviceInfo->input << deviceInfo->output;
        
        //If we found an input device
        if (deviceInfo->input)
        {
            inputDeviceInfo = deviceInfo;
            inputDevIndex = i;
            
            //Reset our output device variables before we look for one.
            outputDeviceInfo = NULL;
            outputDevIndex = -1;
            
            //Search for the corresponding output device and then create a new MidiDevice for them.
            for (int j = 0; j < iNumDevices; j++)
            {
                deviceInfo = Pm_GetDeviceInfo(j);
                
                //If the name of the output devices matches the input device (and our alleged output device
                //really is an output device), aggregate!
                if (strcmp(deviceInfo->name, inputDeviceInfo->name) == 0 && deviceInfo->output)
                {
                    outputDeviceInfo = deviceInfo;
                    outputDevIndex = j;
                    
                    qDebug() << "Found matching output MIDI device";
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

        if (deviceInfo->input || deviceInfo->output)
        {
            //MidiDevicePortMidi *currentDevice = new MidiDevicePortMidi(new MidiControlProcessor(NULL), deviceInfo, i);
            //m_devices.push_back((MidiDevice*)currentDevice);
        }
    }
}

/** Open whatever MIDI devices are selected in the preferences. */
int MidiDeviceManager::setupDevices()
{
    int err = 0;
    QList<MidiDevice*> deviceList = getDeviceList(false, true);
    QListIterator<MidiDevice*> it(deviceList);
    
    qDebug() << "MidiDeviceManager::setupDevices()";
    
    
    while (it.hasNext())
    {
        MidiDevice *cur= it.next();
        MidiMapping *mapping = cur->getMidiMapping();
        QString name = cur->getName();
        
        if ( m_pConfig->getValueString(ConfigKey("[Midi]", name.replace(" ", "_"))) != "1" )
            continue;
        
        qDebug() << "Opening Device:" << cur->getName();
        
        
        cur->close();
        cur->open();
        
        mapping->loadPreset(
                QDir::homePath().append("/").append(SETTINGS_PATH).
                append(name.right(name.size()-3).replace(" ", "-") + ".midi.xml"),
                true
        );
        
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
    dir.setNameFilter("*.midi.xml *.MIDI.XML");

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
