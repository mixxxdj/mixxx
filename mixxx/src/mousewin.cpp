/***************************************************************************
                          mousewin.cpp  -  description
                             -------------------
    begin                : Sat Oct 9 2004
    copyright            : (C) 2004 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mousewin.h"
#include "controlobject.h"
#include "mathstuff.h"
#include <setupapi.h>
#include <initguid.h>
#include <qstring.h>

// define the PowerMate GUID
// {FC3DA4B7-1E9D-47f4-A7E3-151B97C163A6}
DEFINE_GUID(POWERMATE_GUID, 0xfc3da4b7, 0x1e9d, 0x47f4, 0xa7, 0xe3, 0x15, 0x1b, 0x97, 0xc1, 0x63, 0xa6);


MouseWin::MouseWin() : Mouse()
{
    m_hFd = INVALID_HANDLE_VALUE;
    m_iId = -1;
}

MouseWin::~MouseWin()
{
}

bool MouseWin::opendev(QString name)
{
    // Get the handle to the mouse device
    m_hFd = GetDeviceViaInterface((LPGUID)&POWERMATE_GUID,siInstCount);
    if(m_hFd == INVALID_HANDLE_VALUE)
    {
        qDebug("No powermate");
        return false;
    }

    qDebug("found powermate %i",m_iInstNo);

    // Start thread
    start();

    // Turn off led
    led_write(0,0,0,0,0);

    return true;
}

void MouseWin::run()
{
    char *pBuffer = new char[6];
    for (int i=0; i<6; i++)
        pBuffer[i] = 0;

    // create the timeout event
    QString qsName("PowerMate2");
    HANDLE hEvent = CreateEvent(NULL , true, false, (const unsigned short *)qsName.latin1() );

    // Overlapped structure required for async reading
    OVERLAPPED overlapped;
    overlapped.Offset = 0;
    overlapped.OffsetHigh = 0;
    overlapped.hEvent = 0;

    // Setup timeouts when reading using ReadFile
    COMMTIMEOUTS commtimeouts;
    commtimeouts.ReadIntervalTimeout = MAXDWORD;
    commtimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    commtimeouts.ReadTotalTimeoutConstant = 5; // Time to wait for requested characters in msec
    SetCommTimeouts(m_hFd, &commtimeouts);

    // If ReadFile has been called without the data fetched, bRead is true
    bool bRead = false;

    while (1)
    {
        DWORD lBytesRead = 0;
        bool success = false;

        if (!bRead)
        {
            //qDebug("r");
            success = ReadFile(m_hFd, pBuffer, 6, &lBytesRead, &overlapped);
            bRead = true;
        }

        if (bRead) // && GetLastError()==ERROR_IO_PENDING)
        {
            //qDebug("o");
            // asynchronous i/o is still in progress
            // wait for completion or interupt
            //WaitForSingleObject(hEvent, 5);

            // check on the results of the asynchronous read
            success = GetOverlappedResult(m_hFd, &overlapped, &lBytesRead, false);

//          if (!success)
//              qDebug("overlap error: %i",GetLastError());

            //ResetEvent(hEvent);
        }

        //qDebug("success %i, read %i",success,lBytesRead);

        // if there was a problem, or the async operation's still pending
        if (success)
        {
            bRead = false;
            if (lBytesRead == 6)
                process_event(pBuffer);
        }
        //
        // Check if led queue is empty
        //

        // Check if we have to turn on led
        if (m_pRequestLed->available()==0)
        {
            (*m_pRequestLed)--;
            led_write(255, 0, 0, 0, 1);
        }
        else
            led_write(0, 0, 0, 0, 0);
    }
}

void MouseWin::closedev()
{
/*
    if (m_iFd>0)
    {
        CloseHandle(m_iFd);

        // Remove id from list
        QValueList<int>::iterator it = sqlOpenDevs.find(m_iId);
        if (it!=sqlOpenDevs.end())
            sqlOpenDevs.remove(it);
    }
*/
    m_hFd = NULL;
    m_iId = -1;
}

void MouseWin::led_write(int iStaticBrightness, int iSpeed, int iTable, int iAsleep, int iAwake)
{
    iStaticBrightness &= 0xFF;

    if(iSpeed < 0)
        iSpeed = 0;
    if(iSpeed > 510)
        iSpeed = 510;
    if(iTable < 0)
        iTable = 0;
    if(iTable > 2)
        iTable = 2;
    iAsleep = !!iAsleep;
    iAwake = !!iAwake;

    DWORD lBytesReturned = 0;

    USHORT sendVal = iStaticBrightness;
    DeviceIoControl(m_hFd, IOCTL_POWERMATE_SET_LED_BRIGHTNESS,
                    &sendVal, sizeof(sendVal),
                    NULL, 0,    // Output
                    &lBytesReturned, NULL);

    if (iAsleep)
    {
        sendVal = iAsleep;
        DeviceIoControl(m_hFd, IOCTL_POWERMATE_PULSE_DURING_SLEEP,
                        &sendVal, sizeof(sendVal),    // 0, 1
                        NULL, 0,    // Output
                        &lBytesReturned, NULL);
    }

    if (iAwake)
    {
        sendVal = iAwake;
        DeviceIoControl(m_hFd, IOCTL_POWERMATE_PULSE_ALWAYS,
                        &sendVal, sizeof(sendVal),    // 0, 1
                        NULL, 0,    // Output
                        &lBytesReturned, NULL);
    }

    sendVal = iSpeed;
    DeviceIoControl(m_hFd, IOCTL_POWERMATE_PULSE_SPEED,
                    &sendVal, sizeof(sendVal),    // 1-24
                    NULL, 0,    // Output
                    &lBytesReturned, NULL);
}

void MouseWin::process_event(char *pEv)
{
    qDebug("process %i,%i,%i,%i,%i,%i", pEv[0],pEv[1],pEv[2],pEv[3],pEv[4],pEv[5]);
    if (pEv[1]>0 || pEv[1]<0)
    {
        // Update knob variables
        m_iKnobVal = pEv[1];
        m_bSendKnobEvent = true;
    }
    else
    {
        // Send event to GUI thread
        if (pEv[0]==1)
            if (m_pControlObjectButton)
                m_pControlObjectButton->queueFromMidi(NOTE_ON, 1);
        else
            if (m_pControlObjectButton)
                m_pControlObjectButton->queueFromMidi(NOTE_OFF, 1);

//      qDebug("PowerMate: Button was %s %i", pEv[1]? "pressed":"released",pEv[1]);
    }
}

HANDLE MouseWin::GetDeviceViaInterface(GUID* pGuid, DWORD instance)
{
    // Get handle to relevant device information set
    HDEVINFO info = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if(info==INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    // Get interface data for the requested instance
    SP_INTERFACE_DEVICE_DATA ifdata;
    ifdata.cbSize = sizeof(ifdata);
    if(!SetupDiEnumDeviceInterfaces(info, NULL, pGuid, instance, &ifdata))
    {
        SetupDiDestroyDeviceInfoList(info);
        return INVALID_HANDLE_VALUE;
    }

    // Get size of symbolic link name
    DWORD ReqLen;
    SetupDiGetDeviceInterfaceDetail(info, &ifdata, NULL, 0, &ReqLen, NULL);
    PSP_INTERFACE_DEVICE_DETAIL_DATA ifDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)(new char[ReqLen]);
    if( ifDetail==NULL)
    {
        SetupDiDestroyDeviceInfoList(info);
        return INVALID_HANDLE_VALUE;
    }

    // Get symbolic link name
    ifDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
    if( !SetupDiGetDeviceInterfaceDetail(info, &ifdata, ifDetail, ReqLen, NULL, NULL))
    {
        SetupDiDestroyDeviceInfoList(info);
        delete ifDetail;
        return INVALID_HANDLE_VALUE;
    }

    // Open file
    HANDLE rv = CreateFile( ifDetail->DevicePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);  //    FILE_ATTRIBUTE_NORMAL
    //if( rv==INVALID_HANDLE_VALUE) rv = NULL;

    delete ifDetail;
    SetupDiDestroyDeviceInfoList(info);
    return rv;
}
