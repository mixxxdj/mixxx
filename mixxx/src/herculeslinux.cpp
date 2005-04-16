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
    m_iJogLeft = -1;
    m_iJogRight = -1;
    m_dJogLeftOld = 0.;
    m_dJogRightOld = 0.;
}

HerculesLinux::~HerculesLinux()
{
}

void HerculesLinux::run()
{
    while (1)
    {
        getNextEvent();
        
        if (m_pControlObjectLeftBtnPlayProxy->get()!=m_bPlayLeft)
        {
            m_bPlayLeft=!m_bPlayLeft;
            led_write(kiHerculesLedLeftPlay, m_bPlayLeft);
        }
        if (m_pControlObjectRightBtnPlayProxy->get()!=m_bPlayRight)
        {
            m_bPlayRight=!m_bPlayRight;
            led_write(kiHerculesLedRightPlay, m_bPlayRight);
        }
        if (m_pControlObjectLeftBtnLoopProxy->get()!=m_bLoopLeft)
        {
            m_bLoopLeft=!m_bLoopLeft;
            led_write(kiHerculesLedLeftCueBtn, m_bLoopLeft);
        }
        if (m_pControlObjectRightBtnLoopProxy->get()!=m_bLoopRight)
        {
            m_bLoopRight=!m_bLoopRight;
            led_write(kiHerculesLedRightCueBtn, m_bLoopRight);
        }
    }
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
        led_write(kiHerculesLedLeftCueBtn, false);
        led_write(kiHerculesLedRightCueBtn, false);
        led_write(kiHerculesLedLeftPlay, false);
        led_write(kiHerculesLedRightPlay, false);
        led_write(kiHerculesLedLeftSync, false);
        led_write(kiHerculesLedRightSync, false);
        led_write(kiHerculesLedLeftHeadphone, false);
        led_write(kiHerculesLedRightHeadphone, false);
        
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
        double r;
        
        r = m_pRotaryLeft->filter(0.);
        if (r!=0. || r!=m_dLeftVolumeOld)
            sendEvent(r, m_pControlObjectLeftJog);
        m_dLeftVolumeOld = r;
            
        r = m_pRotaryRight->filter(0.);
        if (r!=0. || r!=m_dRightVolumeOld)
            sendEvent(r, m_pControlObjectRightJog);
        m_dRightVolumeOld = r;
        
        return;
    }
      
//     qDebug("v %i, usec %i, isset %i",v,tv.tv_usec,FD_ISSET(m_iFd, &fdset));
      
    struct input_event ev;
    int iR = read(m_iFd, &ev, sizeof(struct input_event));
    if (iR == sizeof(struct input_event))
    {
        double v = 127.*(double)ev.value/256.;
        
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
                    //v=v*4.;
                    dDiff = v-m_dLeftVolumeOld;
                    // qDebug("v %f, diff %f",v,dDiff);
                    if (dDiff>100.)
                        v = 0.;
                    else if (dDiff<-100.)
                        v = 127.;
                    m_dLeftVolumeOld = v;
                    sendEvent(v*4, m_pControlObjectLeftVolume);
                    break;
                case kiHerculesLeftPitch:
                    sendEvent(v, m_pControlObjectLeftPitch);
                    break;
                case kiHerculesLeftJog:
                    iDiff = 0;
                    if (m_iJogLeft>=0)
                        iDiff = ev.value-m_iJogLeft;
                    if (iDiff<-200)
                        iDiff += 256;
                    else if (iDiff>200)
                        iDiff -= 256;
                    m_iJogLeft = ev.value;
//                     qDebug("idiff %i",iDiff);
                    dDiff = m_pRotaryLeft->filter((double)iDiff/16.);
                    
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
                    //v=v*4.;
                    dDiff = v-m_dRightVolumeOld;
                    if (dDiff>100.)
                        v = 0.;
                    else if (dDiff<-100.)
                        v = 127.;
                    m_dRightVolumeOld = v;
                    sendEvent(v*4, m_pControlObjectRightVolume);
                    break;
                case kiHerculesRightPitch:
                    sendEvent(v, m_pControlObjectRightPitch);
                    break;
                case kiHerculesRightJog:
                    iDiff = 0;
                    if (m_iJogRight>=0)
                        iDiff = ev.value-m_iJogRight;
                    if (iDiff<-200)
                        iDiff += 256;
                    else if (iDiff>200)
                        iDiff -= 256;
                    m_iJogRight = ev.value;
                    dDiff = m_pRotaryRight->filter((double)iDiff/16.);
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
                    //m_bCueLeft = !m_bCueLeft;
                    //led_write(kiHerculesLedLeftCueBtn, m_bCueLeft);
                    break;
                case kiHerculesLeftBtnPlay:
                    sendButtonEvent(true, m_pControlObjectLeftBtnPlay);
//                    m_bPlayLeft = !m_bPlayLeft;
//                    led_write(kiHerculesLedLeftPlay, m_bPlayLeft);
                    break;
                case kiHerculesLeftBtnAutobeat:
                    sendButtonEvent(true, m_pControlObjectLeftBtnAutobeat);
                    m_bSyncLeft = !m_bSyncLeft;
//                     led_write(kiHerculesLedLeftSync, m_bSyncLeft);
                    break;
                case kiHerculesLeftBtnMasterTempo:
//                     sendEvent(0, m_pControlObjectLeftBtnMasterTempo);
//                     m_bMasterTempoLeft = !m_bMasterTempoLeft;
//                     led_write(kiHerculesLedLeftMasterTempo, m_bMasterTempoLeft);
                    break;
                case kiHerculesLeftBtn1:
                    m_iLeftFxMode = 0;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(true, m_pControlObjectLeftBtn1);
                    break;
                case kiHerculesLeftBtn2:
                    m_iLeftFxMode = 1;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(true, m_pControlObjectLeftBtn2);
                    break;
                case kiHerculesLeftBtn3:
                    m_iLeftFxMode = 2;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(true, m_pControlObjectLeftBtn3);
                    break;
                case kiHerculesLeftBtnFx:
                    sendButtonEvent(true, m_pControlObjectLeftBtnFx);
/*
                    m_iLeftFxMode = (m_iLeftFxMode+1)%3;
                    qDebug("left fx %i,%i,%i",m_iLeftFxMode==0,m_iLeftFxMode==1,m_iLeftFxMode==2);
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    led_write(kiHerculesLedLeftFx, m_iLeftFxMode==0); 
                    led_write(kiHerculesLedLeftCueLamp, m_iLeftFxMode==1); 
                    led_write(kiHerculesLedLeftLoop, m_iLeftFxMode==2); 
*/
                    break;
                case kiHerculesLeftBtnHeadphone:
                    sendButtonEvent(true, m_pControlObjectLeftBtnHeadphone);
                    m_bHeadphoneLeft = !m_bHeadphoneLeft;
                    led_write(kiHerculesLedLeftHeadphone, m_bHeadphoneLeft);
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
                    //m_bCueRight = !m_bCueRight;
                    //led_write(kiHerculesLedRightCueBtn, m_bCueRight);
                    break;
                case kiHerculesRightBtnPlay:
                    sendButtonEvent(true, m_pControlObjectRightBtnPlay);
//                     m_bPlayRight = !m_bPlayRight;
//                     led_write(kiHerculesLedRightPlay, m_bPlayRight);
                    break;
                case kiHerculesRightBtnAutobeat:
                    sendButtonEvent(true, m_pControlObjectRightBtnAutobeat);
                    m_bSyncRight = !m_bSyncRight;
//                     led_write(kiHerculesLedRightSync, m_bSyncRight);
                    break;
                case kiHerculesRightBtnMasterTempo:
//                     sendEvent(1., m_pControlObjectRightBtnMasterTempo);
//                     m_bMasterTempoRight = !m_bMasterTempoRight;
//                     led_write(kiHerculesLedRightMasterTempo, m_bMasterTempoRight);
                    break;
                case kiHerculesRightBtn1:
                    m_iRightFxMode = 0;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(true, m_pControlObjectRightBtn1);
                    break;
                case kiHerculesRightBtn2:
                    m_iRightFxMode = 1;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(true, m_pControlObjectRightBtn2);
                    break;
                case kiHerculesRightBtn3:
                    m_iRightFxMode = 2;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(true, m_pControlObjectRightBtn3);
                    break;
                case kiHerculesRightBtnFx:
                    sendButtonEvent(true, m_pControlObjectRightBtnFx);
/*
                    m_iRightFxMode = (m_iRightFxMode+1)%3;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    
//                     if (m_iRightFxMode==0)
                    {
                        led_write(kiHerculesLedRightCueLamp, false); 
                        led_write(kiHerculesLedRightLoop, false); 
                        led_write(kiHerculesLedRightFx, false); 
                        led_write(kiHerculesLedRightCueLamp, false); 
                        led_write(kiHerculesLedRightLoop, false); 
                        led_write(kiHerculesLedRightFx, false); 
                    }
                    if (m_iRightFxMode==1)
                        led_write(kiHerculesLedRightCueLamp, true); 
                    if (m_iRightFxMode==2)
                        led_write(kiHerculesLedRightLoop, true); 
*/
                    
                    break;
                case kiHerculesRightBtnHeadphone:
                    sendButtonEvent(true, m_pControlObjectRightBtnHeadphone);
                    m_bHeadphoneRight = !m_bHeadphoneRight;
                    led_write(kiHerculesLedRightHeadphone, m_bHeadphoneRight);
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
//                     m_bCueLeft = !m_bCueLeft;
//                     led_write(kiHerculesLedLeftCueBtn, m_bCueLeft);
                    sendButtonEvent(false, m_pControlObjectLeftBtnCue);
                    break;
                case kiHerculesLeftBtnPlay:
                    sendButtonEvent(false, m_pControlObjectLeftBtnPlay);
                    break;
                case kiHerculesLeftBtnAutobeat:
                    sendButtonEvent(false, m_pControlObjectLeftBtnAutobeat);
                    break;
                case kiHerculesLeftBtnMasterTempo:
//                     sendButtonEvent(false, m_pControlObjectLeftBtnMasterTempo);
                    break;
                case kiHerculesLeftBtn1:
                    m_iLeftFxMode = 0;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(false, m_pControlObjectLeftBtn1);
                    break;
                case kiHerculesLeftBtn2:
                    m_iLeftFxMode = 0;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(false, m_pControlObjectLeftBtn2);
                    break;
                case kiHerculesLeftBtn3:
                    m_iLeftFxMode = 0;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(false, m_pControlObjectLeftBtn3);
                    break;
                case kiHerculesLeftBtnFx:
                    sendButtonEvent(false, m_pControlObjectLeftBtnFx);
                    break;
                case kiHerculesLeftBtnHeadphone:
                    sendButtonEvent(false, m_pControlObjectLeftBtnHeadphone);
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
//                     m_bCueRight = !m_bCueRight;
//                     led_write(kiHerculesLedRightCueBtn, m_bCueRight);
//                     sendButtonEvent(false, m_pControlObjectRightBtnCue);
                    break;
                case kiHerculesRightBtnPlay:
                    sendButtonEvent(false, m_pControlObjectRightBtnPlay);
                    break;
                case kiHerculesRightBtnAutobeat:
                    sendButtonEvent(false, m_pControlObjectRightBtnAutobeat);
                    break;
                case kiHerculesRightBtnMasterTempo:
//                     sendButtonEvent(false, m_pControlObjectRightBtnMasterTempo);
                    break;
                case kiHerculesRightBtn1:
                    m_iRightFxMode = 0;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(false, m_pControlObjectRightBtn1);
                    break;
                case kiHerculesRightBtn2:
                    m_iRightFxMode = 0;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(false, m_pControlObjectRightBtn2);
                    break;
                case kiHerculesRightBtn3:
                    m_iRightFxMode = 0;
                    changeJogMode(m_iLeftFxMode,m_iRightFxMode);
                    sendButtonEvent(false, m_pControlObjectRightBtn3);
                    break;
                case kiHerculesRightBtnFx:
                    sendButtonEvent(false, m_pControlObjectRightBtnFx);
                    break;
                case kiHerculesRightBtnHeadphone:
                    sendButtonEvent(false, m_pControlObjectRightBtnHeadphone);
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
    //if (m_pRequestLed->available()==0)
//     {
      //  (*m_pRequestLed)--;
//         led_write(ki);

//         msleep(5);

//         led_write(0, 0, 0, 0, 0);
//     }
    //else if (iR != sizeof(struct input_event))
    //    msleep(5);
}

void HerculesLinux::led_write(int iLed, bool bOn)
{
//     if (bOn) qDebug("true");
//     else qDebug("false");

    struct input_event ev;
    memset(&ev, 0, sizeof(struct input_event));

    ev.type = EV_LED;
    ev.code = iLed;
    if (bOn)
        ev.value = 3;
    else
        ev.value = 0;

    if (write(m_iFd, &ev, sizeof(struct input_event)) != sizeof(struct input_event))
        qDebug("Hercules: write(): %s", strerror(errno));
}

void HerculesLinux::selectMapping(QString qMapping)
{
    Hercules::selectMapping(qMapping);
    
    if (qMapping==kqInputMappingHerculesInBeat)
    {
        led_write(kiHerculesLedLeftSync, true);
        led_write(kiHerculesLedRightSync, true);
    }
    else
    {    
        led_write(kiHerculesLedLeftSync, false);
        led_write(kiHerculesLedRightSync, false);
    }    
}
