/***************************************************************************
                          portmidienumerator.cpp
                    PortMIDI Device Enumerator Class
                    --------------------------------
    begin                : Thu Feb 25 2010
    copyright            : (C) 2010 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "midideviceportmidi.h"
#include "portmidienumerator.h"

PortMidiEnumerator::PortMidiEnumerator() : MidiDeviceEnumerator() {

}

PortMidiEnumerator::~PortMidiEnumerator() {
    qDebug() << "Deleting PortMIDI devices...";
    QListIterator<MidiDevice*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }
}

/** Enumerate the MIDI devices 
  * This method needs a bit of intelligence because PortMidi (and the underlying MIDI APIs) like to split
  * output and input into separate devices. Eg. PortMidi would tell us the Hercules is two half-duplex devices.
  * To help simplify a lot of code, we're going to aggregate these two streams into a single full-duplex device.
  */
QList<MidiDevice*> PortMidiEnumerator::queryDevices() {
    qDebug() << "Scanning PortMIDI devices:";

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
    return m_devices;
}