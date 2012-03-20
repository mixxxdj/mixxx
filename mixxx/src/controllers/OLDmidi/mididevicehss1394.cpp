/**
  * @file MidiDeviceHss1394.cpp
  * @author Sean M. Pappalardo	spappalardo@mixxx.org
  * @date Thu Feb 25 2010
  * @brief HSS1394-based MIDI backend
  *
  * MidiDeviceHss1394 is a class representing a physical HSS1394 device.
  * (HSS1394 is simply a way to send MIDI messages at high speed over
  * IEEE1394 (FireWire))
  * It uses the HSS1394 API to send and receive MIDI messages to/from
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
#include "mididevicehss1394.h"

// HSS1394 Channel listener stuff

DeviceChannelListener::DeviceChannelListener(int id, QString name, MidiDevice* midiDevice) : hss1394::ChannelListener() {
    m_iId = id;
    m_sName = name;
    m_pMidiDevice = midiDevice;
}

void DeviceChannelListener::Process(const hss1394::uint8 *pBuffer, hss1394::uint uBufferSize) {
    // Called when data has arrived.
	//! This call will occur inside a separate thread.

    unsigned int i = 0;

    // If multiple three-byte messages arrive right next to each other, handle them all
    while (i<uBufferSize) {
        unsigned char status = pBuffer[i];
        unsigned char opcode = status & 0xF0;
        unsigned char channel = status & 0x0F;
        unsigned char note;
        unsigned char velocity;
        switch (status & 0xF0) {
            case MIDI_STATUS_NOTE_OFF:
            case MIDI_STATUS_NOTE_ON:
            case MIDI_STATUS_AFTERTOUCH:
            case MIDI_STATUS_CC:
            case MIDI_STATUS_PITCH_BEND:
                note = pBuffer[i+1];
                velocity = pBuffer[i+2];

                m_pMidiDevice->receive((MidiStatusByte)status, channel, note, velocity);
                i+=3;
                break;
            default:
                // Handle platter messages and any others that are not 3 bytes
                m_pMidiDevice->receive(pBuffer, uBufferSize);
                i=uBufferSize;
                break;
        }
    }
}

void DeviceChannelListener::Disconnected() {
    qDebug()<<"HSS1394 device" << m_sName << "disconnected";
}

void DeviceChannelListener::Reconnected() {
    qDebug()<<"HSS1394 device" << m_sName << "re-connected";
}

// Main MidiDeviceHss1394 code

QMutex MidiDeviceHss1394::m_sHSSLock;   // HSS1394 is not thread-safe

MidiDeviceHss1394::MidiDeviceHss1394(MidiMapping* mapping,
                                        const hss1394::TNodeInfo deviceInfo,
                                        int deviceIndex)
                                        : MidiDevice(mapping)
{
    m_deviceInfo = deviceInfo;
    m_iDeviceIndex = deviceIndex;

    //Note: We prepend the input stream's index to the device's name to prevent duplicate devices from causing mayhem.
    m_strDeviceName = QString("H%1. %2").arg(QString::number(m_iDeviceIndex), QString(deviceInfo.sName.c_str()));

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

    if (m_bIsOpen) {
        qDebug() << "HSS1394 device" << m_strDeviceName << "already open";
        return -1;
    }

    setReceiveInhibit(false);

    startup();

    if (m_strDeviceName == MIXXX_HSS1394_NO_DEVICE_STRING)
        return -1;

    if (midiDebugging()) qDebug() << "MidiDeviceHss1394: Opening" << m_strDeviceName << "index" << m_iDeviceIndex;

    using namespace hss1394;

    m_sHSSLock.lock();
    m_pChannel = Node::Instance()->OpenChannel(m_iDeviceIndex);
    m_sHSSLock.unlock();
    if( m_pChannel == NULL )
    {
        qDebug() << "HSS1394 device" << m_strDeviceName << "could not be opened";
        m_pChannelListener = NULL;
        return -1;
    }

    m_pChannelListener = new DeviceChannelListener(m_iDeviceIndex, m_strDeviceName, this);
    if (false == m_pChannel->InstallChannelListener(m_pChannelListener)) {
        qDebug() << "HSS1394 channel listener could not be installed for device" << m_strDeviceName;
	    delete m_pChannelListener;
        m_pChannelListener = NULL;
		m_pChannel = NULL;
	}

    if (m_pChannel != NULL && m_strDeviceName.contains("SCS.1d",Qt::CaseInsensitive)) {
        // If we are an SCS.1d, set the record encoder event timer to fire at 1ms intervals
        //  to match the 1ms scratch timer in the MIDI script engine
        //
        // By default on f/w version 1.25, a new record encoder event (one new position)
        //  is sent at 500 Hz max, 2ms. When this event occurs, a second timer is reset.
        //  By default this second timer expires periodically at 60 Hz max, around 16.6ms.

        int iPeriod = 60000/1000;   // 1000Hz = 1ms. (Internal clock is 60kHz.)
        int iTimer = 3; // 3 for new event timer, 4 for second “same position repeated” timer
        if (m_pChannel->SendUserControl(iTimer, (const hss1394::uint8*)&iPeriod, 3) == 0)
            qWarning() << "Unable to set SCS.1d platter timer period.";
    }

    m_bIsOpen = true;

    return 0;

}

int MidiDeviceHss1394::close()
{
    setReceiveInhibit(true);    // Prevent deadlock

    if (!m_bIsOpen) {
        qDebug() << "HSS1394 device" << m_strDeviceName << "already closed";
        return -1;
    }

    shutdown();

    //shutdown() locks so we must lock after it.
    QMutexLocker Locker(&m_mutex);

    // Clean up the HSS1394Node
    using namespace hss1394;
    m_sHSSLock.lock();
    if (!Node::Instance()->ReleaseChannel(m_pChannel)) {
        qDebug() << "HSS1394 device" << m_strDeviceName << "could not be released";
        return -1;
    }
    if (m_pChannelListener != NULL) {
		delete m_pChannelListener;
        m_pChannelListener = NULL;
	}
    m_sHSSLock.unlock();

    m_bIsOpen = false;

    return 0;
}

void MidiDeviceHss1394::sendShortMsg(unsigned int word) {
    QMutexLocker Locker(&m_mutex);

    unsigned char data[2];
    data[0] = word & 0xFF;
    data[1] = (word >> 8) & 0xFF;
    data[2] = (word >> 16) & 0xFF;

    QString message = QString("%1 %2 %3").arg(
        QString("%1").arg(data[0], 2, 16, QChar('0')),
        QString("%1").arg(data[1], 2, 16, QChar('0')),
        QString("%1").arg(data[2], 2, 16, QChar('0')));

    m_sHSSLock.lock();
    int bytesSent = m_pChannel->SendChannelBytes(data, 3);

    //if (bytesSent != 3) {
    //    qDebug()<<"ERROR: Sent" << bytesSent << "of 3 bytes:" << message;
    //    //m_pChannel->Flush();
    //}
    m_sHSSLock.unlock();
}

// The sysex data must already contain the start byte 0xf0 and the end byte 0xf7.
void MidiDeviceHss1394::sendSysexMsg(unsigned char data[], unsigned int length)
{
    QMutexLocker Locker(&m_mutex);

    m_sHSSLock.lock();
    int bytesSent = m_pChannel->SendChannelBytes(data,length);

    //if (bytesSent != length) {
    //    qDebug()<<"ERROR: Sent" << bytesSent << "of" << length << "bytes (SysEx)";
    //    //m_pChannel->Flush();
    //}
    m_sHSSLock.unlock();
}
