/***************************************************************************
                          powermatelinux.cpp  -  description
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

#include "powermatelinux.h"
#include <linux/input.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "controleventmidi.h"
#include "controlobject.h"
#include "mathstuff.h"

#ifndef MSC_PULSELED
/* this may not have made its way into the kernel headers yet ... */
#define MSC_PULSELED 0x01
#endif

QValueList <int> PowerMateLinux::sqlOpenDevs;

PowerMateLinux::PowerMateLinux(ControlObject *pControl) : PowerMate(pControl)
{
    m_iFd = -1;
    m_iId = -1;
}

PowerMateLinux::~PowerMateLinux()
{
}

bool PowerMateLinux::opendev()
{
    for(int i=0; i<kiPowermateNumEventDevices; i++)
    {
        if (sqlOpenDevs.find(i)==sqlOpenDevs.end())
        {
            m_iFd = opendev(i);
            if(m_iFd >= 0)
                break;
        }
    }
    
    if (m_iFd>0)
    {
        // Start thread
        start();

        // Turn off led
        led_write(0,0,0,0,0);

        return true;
    }
    else
        return false;
}

void PowerMateLinux::run()
{
    struct input_event buffer[kiPowermateBufferSize];
    int iR, iEvents, i;

    timeval *waittime = new timeval;
    while (1)
    {
        iR = read(m_iFd, buffer, sizeof(struct input_event) * kiPowermateBufferSize);
        if(iR > 0)
        {
            iEvents = iR / sizeof(struct input_event);
            for(i=0; i<iEvents; i++)
                process_event(&buffer[i]);
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
            msleep(1);
   
            led_write(0, 0, 0, 0, 0);
        }
        else
        {
            // Sleep
            msleep(5);
        }
    }
}
    
int PowerMateLinux::opendev(int iId)
{
    char rgcDevName[256];
    sprintf(rgcDevName, "/dev/input/event%d", iId);
    int iFd = open(rgcDevName, O_RDWR|O_NONBLOCK);
    int i;
    char rgcName[255];

    if(iFd < 0)
        return -1;
    if(ioctl(iFd, EVIOCGNAME(sizeof(rgcName)), rgcName) < 0)
    {
        close(iFd);
        return -1;
    }
    // it's the correct device if the prefix matches what we expect it to be:
    for(i=0; i<kiPowermateNumValidPrefixes; i++)
        if(!strncasecmp(rgcName, pPowermateValidPrefix[i], strlen(pPowermateValidPrefix[i])))
        {
            m_iId = iId;
            m_iInstNo = sqlOpenDevs.count();

            // Add id to list of open devices
            sqlOpenDevs.append(iId);
            
            return iFd;
        }

    close(iFd);
    return -1;
}

void PowerMateLinux::closedev()
{
    if (m_iFd>0)
    {
        close(m_iFd);

        // Remove id from list
        QValueList<int>::iterator it = sqlOpenDevs.find(m_iId);
        if (it!=sqlOpenDevs.end())
            sqlOpenDevs.remove(it);
    }
    m_iFd = -1;
    m_iId = -1;
}        

void PowerMateLinux::led_write(int iStaticBrightness, int iSpeed, int iTable, int iAsleep, int iAwake)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(struct input_event));

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

    ev.type = EV_MSC;
    ev.code = MSC_PULSELED;
    ev.value = iStaticBrightness | (iSpeed << 8) | (iTable << 17) | (iAsleep << 19) | (iAwake << 20);

    if(write(m_iFd, &ev, sizeof(struct input_event)) != sizeof(struct input_event))
        qDebug("PowerMate: write(): %s", strerror(errno));
}

void PowerMateLinux::process_event(struct input_event *pEv)
{
    switch(pEv->type)
    {
    case EV_REL:
        if(pEv->code == REL_DIAL)
        {
            // Update knob variables
            m_iKnobVal = pEv->value;
            m_bSendKnobEvent = true;
        }
        break;
    case EV_KEY:
        if(pEv->code == BTN_0)
        {
            // Send event to GUI thread
            if (pEv->value==1)
                QApplication::postEvent(m_pControl,new ControlEventMidi(NOTE_ON, kiPowermateMidiChannel, (char)(m_iInstNo*2+kiPowermateMidiBtn),1));
            else
                QApplication::postEvent(m_pControl,new ControlEventMidi(NOTE_OFF, kiPowermateMidiChannel, (char)(m_iInstNo*2+kiPowermateMidiBtn),1));

//            qDebug("PowerMate: Button was %s %i", ev->value? "pressed":"released",ev->value);
        }
        break;
    }
}
