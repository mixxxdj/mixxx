/***************************************************************************
                          powermatewin.cpp  -  description
                             -------------------
    begin                : Tue Apr 29 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#include "powermatewin.h"
#include "controleventmidi.h"
#include "controlobject.h"
#include "mathstuff.h"
#include <setupapi.h>

int PowerMateWin::siInstCount = 0;

PowerMateWin::PowerMateWin(ControlObject *pControl) : PowerMate(pControl)
{
    m_hFd = NULL;
    m_iId = -1;
}

PowerMateWin::~PowerMateWin()
{
}

bool PowerMateWin::opendev()
{
	// Get the handle to the PowerMate device
	m_hFd = GetDeviceViaInterface((LPGUID)&POWERMATE_GUID,0);
	if(m_hFd == NULL)
		return false;

	// Start thread
    start();

    // Turn off led
    led_write(0,0,0,0,0);

	return true;
}

void PowerMateWin::run()
{
    while (1)
    {
		unsigned char *pBuffer = new unsigned char[8*kiPowermateBufferSize];
		LPDWORD pNoRead;
        if (ReadFile(m_hFd, pBuffer, 8*kiPowermateBufferSize, pNoRead, NULL))
		{
			for (int i=*pNoRead; i>0; i-=8)
				process_event(&pBuffer[i]);
		}
		
		//
        // Check if led queue is empty
        //

        // If last event was a knob event, send out zero value of knob
        if (m_bSendKnobEvent)
            knob_event();

        // Check if we have to turn on led
        if (m_pRequestLed->available()==0)
        {
            (*m_pRequestLed)--;
            led_write(255, 0, 0, 0, 1);

            // Sleep
			// ***
			
            led_write(0, 0, 0, 0, 0);
        }
        else
        {
            // Sleep
            // ***
        }
    }
}
    
void PowerMateWin::closedev()
{
/*
    if (m_iFd>0)
    {
        close(m_iFd);

        // Remove id from list
        QValueList<int>::iterator it = sqlOpenDevs.find(m_iId);
        if (it!=sqlOpenDevs.end())
            sqlOpenDevs.remove(it);
    }
*/
    m_hFd = NULL;
    m_iId = -1;
}        

void PowerMateWin::led_write(int iStaticBrightness, int iSpeed, int iTable, int iAsleep, int iAwake)
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

	LPDWORD lpBytesReturned;

	USHORT sendVal = iStaticBrightness;
	DeviceIoControl(m_hFd, IOCTL_POWERMATE_SET_LED_BRIGHTNESS,
					&sendVal, sizeof(sendVal),
					NULL, 0,    // Output
					lpBytesReturned, NULL);

	if (iAsleep)
	{
		sendVal = iAsleep;
		DeviceIoControl(m_hFd, IOCTL_POWERMATE_PULSE_DURING_SLEEP,
						&sendVal, sizeof(sendVal),    // 0, 1
						NULL, 0,    // Output
						lpBytesReturned, NULL);
	}

	if (iAwake)
	{
		sendVal = iAwake;
		DeviceIoControl(m_hFd, IOCTL_POWERMATE_PULSE_ALWAYS,
						&sendVal, sizeof(sendVal),    // 0, 1
						NULL, 0,    // Output
						lpBytesReturned, NULL);
	}

	sendVal = iSpeed;
	DeviceIoControl(m_hFd, IOCTL_POWERMATE_PULSE_SPEED,
					&sendVal, sizeof(sendVal),    // 1-24
					NULL, 0,    // Output
					lpBytesReturned, NULL);
}

void PowerMateWin::process_event(unsigned char *pEv)
{
	if (pEv[0]&0xf0!=0xf0)
    {
        // Update knob variables
        m_iKnobVal = pEv[1];
        m_bSendKnobEvent = true;
    }
    else
    {
        // Send event to GUI thread
        if (pEv[1]==1)
            QApplication::postEvent(m_pControl,new ControlEventMidi(NOTE_ON, kiPowermateMidiChannel, (char)(m_iInstNo*2+kiPowermateMidiBtn),1));
        else
            QApplication::postEvent(m_pControl,new ControlEventMidi(NOTE_OFF, kiPowermateMidiChannel, (char)(m_iInstNo*2+kiPowermateMidiBtn),1));

//            qDebug("PowerMate: Button was %s %i", ev->value? "pressed":"released",ev->value);
    }
}

HANDLE PowerMateWin::GetDeviceViaInterface(GUID* pGuid, DWORD instance)
{
	// Get handle to relevant device information set
	HDEVINFO info = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if(info==INVALID_HANDLE_VALUE)
		return NULL;

	// Get interface data for the requested instance
	SP_INTERFACE_DEVICE_DATA ifdata;
	ifdata.cbSize = sizeof(ifdata);
	if(!SetupDiEnumDeviceInterfaces(info, NULL, pGuid, instance, &ifdata))
	{
		SetupDiDestroyDeviceInfoList(info);
		return NULL;
	}

	// Get size of symbolic link name
	DWORD ReqLen;
	SetupDiGetDeviceInterfaceDetail(info, &ifdata, NULL, 0, &ReqLen, NULL);
	PSP_INTERFACE_DEVICE_DETAIL_DATA ifDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)(new char[ReqLen]);
	if( ifDetail==NULL)
	{
		SetupDiDestroyDeviceInfoList(info);
		return NULL;
	}

	// Get symbolic link name
	ifDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	if( !SetupDiGetDeviceInterfaceDetail(info, &ifdata, ifDetail, ReqLen, NULL, NULL))
	{
		SetupDiDestroyDeviceInfoList(info);
		delete ifDetail;
		return NULL;
	}

	// Open file
	HANDLE rv = CreateFile( ifDetail->DevicePath, 
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  //  FILE_FLAG_OVERLAPPED  FILE_ATTRIBUTE_NORMAL
	if( rv==INVALID_HANDLE_VALUE) rv = NULL;

	delete ifDetail;
	SetupDiDestroyDeviceInfoList(info);
	return rv;
}
