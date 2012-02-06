/**
  * @file hidcontroller.cpp
  * @author Sean M. Pappalardo	spappalardo@mixxx.org
  * @date Sun May 1 2011
  * @brief HID controller backend
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

#include "hidcontroller.h"

// ------ HID controller reader thread ------

HidReader::HidReader(hid_device* device) : QThread() {
    m_pHidDevice = device;
}

HidReader::~HidReader() {
}

void HidReader::run() {
    unsigned char *data = new unsigned char[255];

    m_bStop=false;
    int result;
    unsigned int length=0;

    while(!m_bStop) {
        // Blocked polling: The only problem with this is that we can't close
        //  the device until the block is released, which means the controller
        //  has to send more data
//         result = hid_read_timeout(m_pHidDevice, data, 255, -1);

        // This relieves that at the cost of higher CPU usage since we only
        //  block for a short while (500ms)
        result = hid_read_timeout(m_pHidDevice, data, 255, 500);
        
        if (result > 0) {
            length=result;
//             qDebug() << "Read" << length << "bytes, pointer:" << data;
            // Copy the array to avoid thread contention
            unsigned char *temp = new unsigned char[length];
            memcpy(temp, data, length);
            emit(incomingData(temp,length));
            // WARNING: Receiving slot must delete[] temp!
        }
    }

    delete data;
}

// ------ HidController ------

HidController::HidController(const hid_device_info deviceInfo) {
    m_deviceInfo = deviceInfo;
    
    //Note: We include the last 4 digits of the serial number and the interface number to
    //  allow the user (and Mixxx!) to keep track of which is which
    m_sDeviceName = QString("%1 %2_%3").arg(QString::fromWCharArray(deviceInfo.product_string))
                    .arg(QString::fromWCharArray(deviceInfo.serial_number).right(4))
                    .arg(QString::number(deviceInfo.interface_number));
    
    // Set the Unique IDentifier to the serial_number plus the interface number
    m_sUID = QString::fromWCharArray(deviceInfo.serial_number);
    m_sUID.append(QString::number(deviceInfo.interface_number));
    
    // All HID devices are full-duplex
    m_bIsInputDevice = true;
    m_bIsOutputDevice = true;

    m_pReader = NULL;
}

HidController::~HidController() {
    close();
}

int HidController::open() {
    if (m_bIsOpen) {
        qDebug() << "HID device" << m_sDeviceName << "already open";
        return -1;
    }

    if (debugging()) qDebug() << "Opening HID device" << m_sDeviceName;
    
    // Open Device
    // FIXME: figure out why trying to open the device including the serial number fails (on Linux at least)
//     m_pHidDevice = hid_open(m_deviceInfo.vendor_id, m_deviceInfo.product_id, m_deviceInfo.serial_number);
    m_pHidDevice = hid_open(m_deviceInfo.vendor_id, m_deviceInfo.product_id, NULL);
    if (m_pHidDevice == NULL) {
        qWarning()  << "Unable to open HID device" << m_sDeviceName;
        return -1;
    }
        
    m_bIsOpen = true;

    startEngine();

    if (m_pReader != NULL) {
        qWarning() << "HidReader already present for"<<m_sDeviceName;
    }
    else {
        m_pReader = new HidReader(m_pHidDevice);
        m_pReader->setObjectName(QString("HidReader %1").arg(m_sDeviceName));

        connect(m_pReader, SIGNAL(incomingData(unsigned char*, unsigned int)),
                this, SLOT(receivePointer(unsigned char*, unsigned int)));

        // Controller input needs to be prioritized since it can affect the audio
        //  directly, like when scratching
        m_pReader->start(QThread::HighPriority);
    }
    
    return 0;
}

int HidController::close() {
    if (!m_bIsOpen) {
        qDebug() << "HID device" << m_sDeviceName << "already closed";
        return -1;
    }
    
    qDebug() << "Shutting down HID device" << m_sDeviceName;

    // Stop the reading thread
    if (m_pReader == NULL) {
        qWarning() << "HidReader not present for"<<m_sDeviceName<<"yet the device is open!";
    }
    else {
        disconnect(m_pReader, SIGNAL(incomingData(unsigned char*, unsigned int)),
                this, SLOT(receivePointer(unsigned char*, unsigned int)));
        m_pReader->stop();
        hid_set_nonblocking(m_pHidDevice, 1);   // Quit blocking
        if (debugging()) qDebug() << "  Waiting on reader to finish";
        m_pReader->wait();
        delete m_pReader;
        m_pReader = NULL;
    }
    
    // Stop controller engine here to ensure it's done before the device is closed
    //  incase it has any final parting messages
    stopEngine();
    
    // Close device
    if (debugging()) qDebug() << "  Closing device";
    hid_close(m_pHidDevice);
    
    m_bIsOpen = false;
    
    return 0;
}

void HidController::send(QList<int> data, unsigned int length, unsigned int reportID) {

    unsigned char * msg;
    msg = new unsigned char [length];

    for (unsigned int i=0; i<length; i++) {
        msg[i] = data.at(i);
    }

    send(msg,length,reportID);
    delete[] msg;
}

void HidController::sendBa(QByteArray data, unsigned int length, unsigned int reportID) {
    
    unsigned char* msg = reinterpret_cast<unsigned char*>(data.data());
    send(msg,length,reportID);
}

void HidController::send(unsigned char data[], unsigned int length, unsigned int reportID) {
    
    // Append the Report ID to the beginning of data[] per the API.
    unsigned char * buffer = (unsigned char*) malloc( sizeof(unsigned char) * (length + 1));
    memcpy(buffer + sizeof(unsigned char), data, length);
    buffer[0] = (unsigned char) reportID;
    
    int result = hid_write(m_pHidDevice, buffer, length+1);
    if (result==-1) qWarning() << "Unable to send data to" << m_sDeviceName << ":" << hid_error(m_pHidDevice);
    else if (debugging()) qDebug() << result << "bytes sent to" << m_sDeviceName << "(including report ID of" << reportID << ")";

    free(buffer);
}
