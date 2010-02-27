/**
  * @file MidiDeviceHss1394.cpp
  * @author Sean M. Pappalardo	spappalardo@mixxx.org
  * @date Thu Feb 25 2010
  * @brief HSS1394-based MIDI backend
  * 
  * MidiDeviceHss1394 is a class representing a physical HSS1394 device.
  * (HSS1394 is simply a way to send MIDI messages at high speed over
  * IEEE1394 (FireWire))
  * It uses the HSS1394 API to send and receive MIDI events to/from
  * the device.
  *
  *
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
#include "configobject.h"
#include "midimapping.h"
#include "MidiDeviceHss1394.h"

QMutex MidiDeviceHss1394::m_sHSSLock;   // HSS1394 is not thread-safe

MidiDeviceHss1394::MidiDeviceHss1394(MidiMapping* mapping, 
                                        const hss1394::TNodeInfo* deviceInfo, 
                                        int deviceIndex)
                                        : MidiDevice(mapping) 
{
    m_bStopRequested = false;
    m_pDeviceInfo = deviceInfo;
    m_iDeviceIndex = deviceIndex;
    
    //Note: We prepend the input stream's index to the device's name to prevent duplicate devices from causing mayhem.
    m_strDeviceName = QString("%1. %2").arg(QString::number(m_iDeviceIndex)).arg(QString(deviceInfo->sName.c_str()));
    
    // All HSS1394 devices are full-duplex
    m_bIsInputDevice = true;
    m_bIsOutputDevice = true;

    m_pMidiMapping->setName(m_strDeviceName);
}

MidiDeviceHss1394::~MidiDeviceHss1394()
{
    close();
}

int MidiDeviceHss1394::open()
{
    QMutexLocker Locker(&m_mutex); //Make this function thread safe.
/*    
    if (m_bIsOpen) {
        qDebug() << "HSS1394 device" << m_strDeviceName << "already open";
        return -1;
    }
    
    startup();
    
    m_bStopRequested = false;
    
    if (m_strDeviceName == MIXXX_HSS1394_NO_DEVICE_STRING)
        return -1;

    m_sHSSLock.lock();
    PmError err = Pm_Initialize();
    m_sHSSLock.unlock();
    if( err != pmNoError )
    {
        qDebug() << "HSS1394 error:" << Pm_GetErrorText(err);
        return -1;
    }

    if (m_pInputDeviceInfo)
    {    
        if (m_bIsInputDevice)
        {
            if (midiDebugging()) qDebug() << "MidiDeviceHss1394: Opening" << m_pInputDeviceInfo->name << "index" << m_iInputDeviceIndex << "for input";

            m_sHSSLock.lock();
            err = Pm_OpenInput( &m_pInputStream,
                    m_iInputDeviceIndex,
                    NULL, //No drive hacks
                    MIXXX_HSS1394_BUFFER_LEN,
                    NULL, 
                    NULL);
            m_sHSSLock.unlock();
    
            if( err != pmNoError )
            {
                qDebug() << "HSS1394 error:" << Pm_GetErrorText(err);
                return -2;
            }
        }
    }
    if (m_pOutputDeviceInfo)
    {
        if (m_bIsOutputDevice)
        {
            if (midiDebugging()) qDebug() << "MidiDeviceHss1394: Opening" << m_pOutputDeviceInfo->name << "index" << m_iOutputDeviceIndex << "for output";
            
            m_sHSSLock.lock();
            err = Pm_OpenOutput( &m_pOutputStream,
                    m_iOutputDeviceIndex,
                    NULL, // No driver hacks
                    0,      // No buffering
                    NULL, // Use PortTime for timing
                    NULL, // No time info
                    0);   // No latency compensation.
            m_sHSSLock.unlock();

            if( err != pmNoError )
            {
                qDebug() << "HSS1394 error:" << Pm_GetErrorText(err);
                return -2;
            }
        }
    }   
    
    m_bIsOpen = true;
    start();
*/    
    return 0;

}

int MidiDeviceHss1394::close()
{    
    if (!m_bIsOpen) {
        qDebug() << "HSS1394 device" << m_strDeviceName << "already closed";
        return -1;
    }
    
    shutdown();

    //shutdown() locks so we must lock after it.
    QMutexLocker Locker(&m_mutex);

    m_bStopRequested = true;
/*    
    if (m_pInputStream)
    {
        m_sHSSLock.lock();
        PmError err = Pm_Close(m_pInputStream);
        if( err != pmNoError )
        {
            qDebug() << "HSS1394 error:" << Pm_GetErrorText(err);
            m_sHSSLock.unlock();
            return -1;
        }
        m_sHSSLock.unlock();
    }
    
    if (m_pOutputStream)
    {
        m_sHSSLock.lock();
        PmError err = Pm_Close(m_pOutputStream);
        if( err != pmNoError )
        {
            qDebug() << "HSS1394 error:" << Pm_GetErrorText(err);
            m_sHSSLock.unlock();
            return -1;
        }
        m_sHSSLock.unlock();
    }
    */
    m_bIsOpen = false;
            
    return 0;
}

void MidiDeviceHss1394::run()
{
    QThread::currentThread()->setObjectName(QString("HSS %1").arg(m_strDeviceName));
    int numEvents = 0;
    bool stopRunning = false;
    
/*
    do
    {
        if (m_pInputStream)
        {
            //TODO: Inhibit receiving of MIDI messages to prevent race condition?
        
            m_sHSSLock.lock();
            numEvents = Pm_Read(m_pInputStream, m_midiBuffer, MIXXX_HSS1394_BUFFER_LEN);
            m_sHSSLock.unlock();
    
            for (int i = 0; i < numEvents; i++)
            {
                //if (Pm_MessageStatus(m_midiBuffer[i].message) == 0x90) //Note on, channel 1
                {
                    unsigned char status = Pm_MessageStatus(m_midiBuffer[i].message);
                    unsigned char opcode = status & 0xF0;
                    unsigned char channel = status & 0x0F;
                    unsigned char note = Pm_MessageData1(m_midiBuffer[i].message);
                    unsigned char velocity = Pm_MessageData2(m_midiBuffer[i].message);
                                    
                    MidiDevice::receive((MidiStatusByte)status, channel, note, velocity);

                }
            }
        }
        
        usleep(5000); //Sleep this thread for 5 milliseconds between checking for new MIDI events.
        
        m_mutex.lock();
        stopRunning = m_bStopRequested; //Cache locally for thread-safety.
        m_mutex.unlock(); //Have to unlock inside the loop to give the other thread a chance to lock.
        
    } while (!stopRunning);
    */

}

void MidiDeviceHss1394::sendShortMsg(unsigned int word) 
{
    QMutexLocker Locker(&m_mutex);
    /*
    if (m_pOutputStream)
    {
        m_sHSSLock.lock();
        PmError err = Pm_WriteShort(m_pOutputStream, 0, word);
        if( err != pmNoError ) qDebug() << "HSS1394 sendShortMsg error:" << Pm_GetErrorText(err);
        m_sHSSLock.unlock();
    }
    */
}

// The sysex data must already contain the start byte 0xf0 and the end byte 0xf7.
void MidiDeviceHss1394::sendSysexMsg(unsigned char data[], unsigned int length) 
{
    QMutexLocker Locker(&m_mutex); 
    /*
    if (m_pOutputStream)
    {
        m_sHSSLock.lock();
        PmError err = Pm_WriteSysEx(m_pOutputStream, 0, data);
        if( err != pmNoError ) qDebug() << "HSS1394 sendSysexMsg error:" << Pm_GetErrorText(err);
        m_sHSSLock.unlock();
    } 
    */
}
