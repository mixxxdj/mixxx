/***************************************************************************
                          herculeslinux.cpp  -  description
                             -------------------
    begin                : Tue Feb 22 2005
    copyright            : (C) 2005 by Tue Haste Andersen
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

#include "herculeslinux.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "controlobject.h"
#include "mathstuff.h"
#include "rotary.h"

#ifndef MSC_PULSELED
  // this may not have made its way into the kernel headers yet ...
  #define MSC_PULSELED 0x01
#endif

QValueList <int> HerculesLinux::sqlOpenDevs;

HerculesLinux::HerculesLinux() : Hercules()
{
    m_iFd = -1;
    m_iId = -1;
    m_iJogLeft = 0;
    m_iJogRight = 0;
}

HerculesLinux::~HerculesLinux()
{
}

void HerculesLinux::run()
{
    while (1)
        getNextEvent();
}

bool HerculesLinux::opendev()
{
    for(int i=0; i<kiHerculesNumEventDevices; i++)
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

void HerculesLinux::closedev()
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

int HerculesLinux::opendev(int iId)
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
    for(i=0; i<kiHerculesNumValidPrefixes; i++)
        if (kqHerculesValidPrefix[i]==rgcName)
        {
            m_iId = iId;
            m_iInstNo = sqlOpenDevs.count();

            // Add id to list of open devices
            sqlOpenDevs.append(iId);

//             qDebug("pm id %i",iId);

            return iFd;
        }

    close(iFd);
    return -1;
}

void HerculesLinux::getNextEvent()
{
    FD_ZERO(&fdset);
    FD_SET(m_iFd, &fdset);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    int v = select(m_iFd+1, &fdset, 0, 0, &tv);
    if (v<=0)
    {
        sendEvent(m_pRotaryLeft->filter(0.), m_pControlObjectLeftJog);
        sendEvent(m_pRotaryRight->filter(0.), m_pControlObjectRightJog);
        return;
    }
      
//     qDebug("v %i, usec %i, isset %i",v,tv.tv_usec,FD_ISSET(m_iFd, &fdset));
      
    struct input_event ev;
    int iR = read(m_iFd, &ev, sizeof(struct input_event));
    if (iR == sizeof(struct input_event))
    {
        double v = (double)ev.value/2.;
        
        //qDebug("type %i, code %i, value %i",ev.type,ev.code,ev.value);
        
        switch(ev.type)
        {
        case EV_ABS:
            //qDebug("code %i",ev.code);
            int iDiff;
            double dDiff;
            
            switch (ev.code)
            {
                case kiHerculesLeftTreble:
                    sendEvent(v, m_pControlObjectLeftTreble);
                    break;
                case kiHerculesLeftMiddle:
                    sendEvent(v, m_pControlObjectLeftMiddle);
                    break;
                case kiHerculesLeftBass:
                    sendEvent(v, m_pControlObjectLeftBass);
                    break;
                case kiHerculesLeftVolume:
                    sendEvent(v, m_pControlObjectLeftVolume);
                    break;
                case kiHerculesLeftPitch:
                    sendEvent(v, m_pControlObjectLeftPitch);
                    break;
                case kiHerculesLeftJog:
                    iDiff = ev.value-m_iJogLeft;
                    if (iDiff<-200)
                        iDiff += 256;
                    else if (iDiff>200)
                        iDiff -= 256;
                    m_iJogLeft = ev.value;
                    dDiff = m_pRotaryLeft->filter((double)iDiff);
                    sendEvent(dDiff, m_pControlObjectLeftJog);
                    break;
                case kiHerculesRightTreble:
                    sendEvent(v, m_pControlObjectRightTreble);
                    break;
                case kiHerculesRightMiddle:
                    sendEvent(v, m_pControlObjectRightMiddle);
                    break;
                case kiHerculesRightBass:
                    sendEvent(v, m_pControlObjectRightBass);
                    break;
                case kiHerculesRightVolume:
                    sendEvent(v, m_pControlObjectRightVolume);
                    break;
                case kiHerculesRightPitch:
                    sendEvent(v, m_pControlObjectRightPitch);
                    break;
                case kiHerculesRightJog:
                    iDiff = ev.value-m_iJogRight;
                    if (iDiff<-200)
                        iDiff += 256;
                    else if (iDiff>200)
                        iDiff -= 256;
                    m_iJogRight = ev.value;
                    dDiff = m_pRotaryRight->filter((double)iDiff);
                    sendEvent(dDiff, m_pControlObjectRightJog);
                    break;
                case kiHerculesCrossfade:
                    sendEvent(v, m_pControlObjectCrossfade);
                    break;                
//                 default:
//                     sendEvent(0., m_pControlObjectLeftJog);
//                     sendEvent(0., m_pControlObjectRightJog);
            }
            break;
        case EV_KEY:
            if (ev.value==1)
            {
                switch (ev.code)
                {
                case kiHerculesLeftBtnPitchBendMinus:
                    sendButtonEvent(true, m_pControlObjectLeftBtnPitchBendMinus);
                    break;
                case kiHerculesLeftBtnPitchBendPlus:
                    sendButtonEvent(true, m_pControlObjectLeftBtnPitchBendPlus);
                    break;
                case kiHerculesLeftBtnTrackNext:
                    sendButtonEvent(true, m_pControlObjectLeftBtnTrackNext);
                    break;
                case kiHerculesLeftBtnTrackPrev:
                    sendButtonEvent(true, m_pControlObjectLeftBtnTrackPrev);
                    break;
                case kiHerculesLeftBtnCue:
                    sendButtonEvent(true, m_pControlObjectLeftBtnCue);
                    break;
                case kiHerculesLeftBtnPlay:
                    sendButtonEvent(true, m_pControlObjectLeftBtnPlay);
                    
                    break;
                case kiHerculesLeftBtnAutobeat:
                    sendButtonEvent(true, m_pControlObjectLeftBtnAutobeat);
                    break;
                case kiHerculesLeftBtnMasterTempo:
                    sendButtonEvent(true, m_pControlObjectLeftBtnMasterTempo);
                    break;
                case kiHerculesLeftBtn1:
                    sendButtonEvent(true, m_pControlObjectLeftBtn1);
                    break;
                case kiHerculesLeftBtn2:
                    sendButtonEvent(true, m_pControlObjectLeftBtn2);
                    break;
                case kiHerculesLeftBtn3:
                    sendButtonEvent(true, m_pControlObjectLeftBtn3);
                    break;
                case kiHerculesLeftBtnFx:
                    sendButtonEvent(true, m_pControlObjectLeftBtnFx);
                    break;
                case kiHerculesRightBtnPitchBendMinus:
                    sendButtonEvent(true, m_pControlObjectRightBtnPitchBendMinus);
                    break;
                case kiHerculesRightBtnPitchBendPlus:
                    sendButtonEvent(true, m_pControlObjectRightBtnPitchBendPlus);
                    break;
                case kiHerculesRightBtnTrackNext:
                    sendButtonEvent(true, m_pControlObjectRightBtnTrackNext);
                    break;
                case kiHerculesRightBtnTrackPrev:
                    sendButtonEvent(true, m_pControlObjectRightBtnTrackPrev);
                    break;
                case kiHerculesRightBtnCue:
                    sendButtonEvent(true, m_pControlObjectRightBtnCue);
                    break;
                case kiHerculesRightBtnPlay:
                    sendButtonEvent(true, m_pControlObjectRightBtnPlay);
                    break;
                case kiHerculesRightBtnAutobeat:
                    sendButtonEvent(true, m_pControlObjectRightBtnAutobeat);
                    break;
                case kiHerculesRightBtnMasterTempo:
                    sendButtonEvent(true, m_pControlObjectRightBtnMasterTempo);
                    break;
                case kiHerculesRightBtn1:
                    sendButtonEvent(true, m_pControlObjectRightBtn1);
                    break;
                case kiHerculesRightBtn2:
                    sendButtonEvent(true, m_pControlObjectRightBtn2);
                    break;
                case kiHerculesRightBtn3:
                    sendButtonEvent(true, m_pControlObjectRightBtn3);
                    break;
                case kiHerculesRightBtnFx:
                    sendButtonEvent(true, m_pControlObjectRightBtnFx);
                    break;
                }
            }
            else
            {
                switch (ev.code)
                {
                case kiHerculesLeftBtnPitchBendMinus:
                    sendButtonEvent(false, m_pControlObjectLeftBtnPitchBendMinus);
                    break;
                case kiHerculesLeftBtnPitchBendPlus:
                    sendButtonEvent(false, m_pControlObjectLeftBtnPitchBendPlus);
                    break;
                case kiHerculesLeftBtnTrackNext:
                    sendButtonEvent(false, m_pControlObjectLeftBtnTrackNext);
                    break;
                case kiHerculesLeftBtnTrackPrev:
                    sendButtonEvent(false, m_pControlObjectLeftBtnTrackPrev);
                    break;
                case kiHerculesLeftBtnCue:
                    sendButtonEvent(false, m_pControlObjectLeftBtnCue);
                    break;
                case kiHerculesLeftBtnPlay:
                    sendButtonEvent(false, m_pControlObjectLeftBtnPlay);
                    break;
                case kiHerculesLeftBtnAutobeat:
                    sendButtonEvent(false, m_pControlObjectLeftBtnAutobeat);
                    break;
                case kiHerculesLeftBtnMasterTempo:
                    sendButtonEvent(false, m_pControlObjectLeftBtnMasterTempo);
                    break;
                case kiHerculesLeftBtn1:
                    sendButtonEvent(false, m_pControlObjectLeftBtn1);
                    break;
                case kiHerculesLeftBtn2:
                    sendButtonEvent(false, m_pControlObjectLeftBtn2);
                    break;
                case kiHerculesLeftBtn3:
                    sendButtonEvent(false, m_pControlObjectLeftBtn3);
                    break;
                case kiHerculesLeftBtnFx:
                    sendButtonEvent(false, m_pControlObjectLeftBtnFx);
                    break;
                case kiHerculesRightBtnPitchBendMinus:
                    sendButtonEvent(false, m_pControlObjectRightBtnPitchBendMinus);
                    break;
                case kiHerculesRightBtnPitchBendPlus:
                    sendButtonEvent(false, m_pControlObjectRightBtnPitchBendPlus);
                    break;
                case kiHerculesRightBtnTrackNext:
                    sendButtonEvent(false, m_pControlObjectRightBtnTrackNext);
                    break;
                case kiHerculesRightBtnTrackPrev:
                    sendButtonEvent(false, m_pControlObjectRightBtnTrackPrev);
                    break;
                case kiHerculesRightBtnCue:
                    sendButtonEvent(false, m_pControlObjectRightBtnCue);
                    break;
                case kiHerculesRightBtnPlay:
                    sendButtonEvent(false, m_pControlObjectRightBtnPlay);
                    break;
                case kiHerculesRightBtnAutobeat:
                    sendButtonEvent(false, m_pControlObjectRightBtnAutobeat);
                    break;
                case kiHerculesRightBtnMasterTempo:
                    sendButtonEvent(false, m_pControlObjectRightBtnMasterTempo);
                    break;
                case kiHerculesRightBtn1:
                    sendButtonEvent(false, m_pControlObjectRightBtn1);
                    break;
                case kiHerculesRightBtn2:
                    sendButtonEvent(false, m_pControlObjectRightBtn2);
                    break;
                case kiHerculesRightBtn3:
                    sendButtonEvent(false, m_pControlObjectRightBtn3);
                    break;
                case kiHerculesRightBtnFx:
                    sendButtonEvent(false, m_pControlObjectRightBtnFx);
                    break;
                
                }
            }
            break;
//         default:
//             sendEvent(0., m_pControlObjectLeftJog);
//             sendEvent(0., m_pControlObjectRightJog);
        }
    }
    else
    {
//         sendEvent(0., m_pControlObjectLeftJog);
//         sendEvent(0., m_pControlObjectRightJog);
    }
            
    //
    // Check if led queue is empty
    //
    // Check if we have to turn on led
    if (m_pRequestLed->available()==0)
    {
        (*m_pRequestLed)--;
        led_write(255, 0, 0, 0, 1);

        msleep(5);

        led_write(0, 0, 0, 0, 0);
    }
    //else if (iR != sizeof(struct input_event))
    //    msleep(5);
}

void HerculesLinux::led_write(int iStaticBrightness, int iSpeed, int iTable, int iAsleep, int iAwake)
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
        qDebug("Hercules: write(): %s", strerror(errno));
}
