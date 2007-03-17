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

/*
 * =========Version History=============
 * Version 1.5.0 Wed Aug 23 2006 - modfications by Garth Dahlstrom <ironstorm@users.sf.net>
 * - works with Hercule DJ Console MK2
 * - fixed control scale to be 0..255 on bass, mid, treb, etc
 * - reverted pitch control back to pitch knob, added PitchChange method to limit pitch change
 * - fixed volume sliders, crossfader
 * - TODO: fix the JogDials to do something like a scratch (they do a temporary pitch bend now :\)
 * - TODO: reset m_iPitchOffsetLeft or m_iPitchOffsetRight value to -9999 when mouse/keyboard adjusts pitch slider (see PitchChange method header)
 * - TODO: figure out how to get the LEDs working 
 */

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
    m_iPitchOffsetLeft=-9999;
    m_iPitchOffsetRight=-9999;
    m_iPitchLeft = 127;
    m_iPitchRight = 127;
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
//            qDebug("Looking for a Hercules DJ Console on /dev/input/event%d ...", i);
            m_iFd = opendev(i);
            if(m_iFd >= 0)
                break;
        }
    }
    if (m_iFd>0)
    {
        qDebug("Hercules device @ %d", m_iFd);
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
        qDebug("Hercules device (%d) not found!", m_iFd);
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

    if(iFd < 0) {
//        qDebug("Could not open Hercules at /dev/input/event%d [%s]",iId, strerror(errno));
        if (errno==13) {
          qDebug("If you have a Hercules device plugged into USB, you'll need to either execute 'sudo chmod o+rw- /dev/input/event?' or run mixxx as root.");
        }         
        return -1;
    }
    if(ioctl(iFd, EVIOCGNAME(sizeof(rgcName)), rgcName) < 0)
    {
        qDebug("EVIOCGNAME got negative size at /dev/input/event%d",iId);
        close(iFd);
        return -1;
    }
    // it's the correct device if the prefix matches what we expect it to be:
    for(i=0; i<kiHerculesNumValidPrefixes; i++) {
        if (kqHerculesValidPrefix[i]==rgcName)
        {
            m_iId = iId;
            m_iInstNo = sqlOpenDevs.count();

            // Add id to list of open devices
            sqlOpenDevs.append(iId);

            qDebug("pm id %i",iId);

            return iFd;
        } 
        qDebug("  %d. rgcName = [%s]",i,(const char*)rgcName);
        qDebug("  %d. kqHerculesValidPrefix[i] = [%s]",i, kqHerculesValidPrefix[i].data());
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
        //double v = 127.*(double)ev.value/256.;
        double v = ((ev.value+1)/(4.- ((ev.value>((7/8.)*256))*((ev.value-((7/8.)*256))*1/16.)))); // GED's formula, might need some work
        //qDebug("type %i, code %i, value %i",ev.type,ev.code,ev.value);
        //qDebug("type %i, code %i, value %i, v is %5.3f",ev.type,ev.code,ev.value,v);
        
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
                    m_dLeftVolumeOld = ev.value/2.;
                    sendEvent(ev.value/2., m_pControlObjectLeftVolume);
                    break;
                case kiHerculesLeftPitch:
                    //qDebug("");
                    sendEvent(PitchChange("Left", ev.value, m_iPitchLeft, m_iPitchOffsetLeft), m_pControlObjectLeftPitch);
                    break;
                case kiHerculesLeftJog:
                    iDiff = 0;
                    if (m_iJogLeft>=0)
                        iDiff = ev.value-m_iJogLeft;
                    if (iDiff<-200)
                        iDiff += 256;
                    else if (iDiff>200)
                        iDiff -= 256;
                    dDiff = m_pRotaryLeft->filter((double)iDiff/16.);
                    //qDebug("Left Jog - ev.value %i, m_iJogLeft %i, idiff %i, dDiff %5.3f",ev.value, m_iJogLeft, iDiff, dDiff);
                    m_iJogLeft = ev.value;
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
                    m_dRightVolumeOld = ev.value/2.;
                    //qDebug("R Volume %5.3f",ev.value/2.);
                    sendEvent(ev.value/2., m_pControlObjectRightVolume);
                    break;
                case kiHerculesRightPitch:
                    //qDebug("");
                    sendEvent(PitchChange("Right", ev.value, m_iPitchRight, m_iPitchOffsetRight), m_pControlObjectRightPitch);
                    break;
                case kiHerculesRightJog:
                    iDiff = 0;
                    if (m_iJogRight>=0)
                        iDiff = ev.value-m_iJogRight;
                    if (iDiff<-200)
                        iDiff += 256;
                    else if (iDiff>200)
                        iDiff -= 256;
                    dDiff = m_pRotaryRight->filter((double)iDiff/16.); 
//                    qDebug("Right Jog - ev.value %i, m_iJogRight %i, idiff %i, dDiff %5.3f",ev.value, m_iJogRight, iDiff, dDiff);
                    m_iJogRight = ev.value;
                    sendEvent(dDiff, m_pControlObjectRightJog);
                    break;
                case kiHerculesCrossfade:
                	//qDebug("(ev.value+1)/2.0f: %f", (ev.value+1)/2.0f);
                    sendEvent((ev.value+1)/2.0f, m_pControlObjectCrossfade);
                    break;                
//              default:
//                  sendEvent(0., m_pControlObjectLeftJog);
//                  sendEvent(0., m_pControlObjectRightJog);
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
                     //led_write(kiHerculesLedRightMasterTempo, m_bMasterTempoRight);
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
                    //led_write(kiHerculesLedRightHeadphone, m_bHeadphoneRight);
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
                    //sendButtonEvent(false, m_pControlObjectLeftBtnTrackNext);
                    break;
                case kiHerculesLeftBtnTrackPrev:
                    //sendButtonEvent(false, m_pControlObjectLeftBtnTrackPrev);
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
                    //sendButtonEvent(false, m_pControlObjectRightBtnTrackNext);
                    break;
                case kiHerculesRightBtnTrackPrev:
                    //sendButtonEvent(false, m_pControlObjectRightBtnTrackPrev);
                    break;
                case kiHerculesRightBtnCue:
//                     m_bCueRight = !m_bCueRight;
//                     led_write(kiHerculesLedRightCueBtn, m_bCueRight);
                     sendButtonEvent(false, m_pControlObjectRightBtnCue);
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
    //if (m_qRequestLed.available()==0)
//     {
      //  m_qRequestLed--;
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



double HerculesLinux::PitchChange(const QString ControlSide, const int ev_value, int &m_iPitchPrevious, int &m_iPitchOffset) {
	// Note: Calling this function with m_iPitchPrevious having a value of -9999 should result in 
	// resetting the pitch offset (this should be when mouse changes or resets the pitch)
	//
	// TODO: 
	//  i) Pitch range should be .25 to 127, as implemented currently it does -.25 to 127.25.
	// ii) On my dev machine P3-800, it is possible to spin the pitch knob fast enough to change by 2, if this happens at 
	//	min/max or rollover, the checks may be skipped resulting in changes to the pitch way outside the allowed 
	//	range (i.e. -33.04). This could be a USB latency problem or it could be normal for the Consoles, test on a 
	//	faster PC, if it happens on a faster PC then try to figure out a fix.
	//iii) Not sure if checks will work properly if the hardware pitch value is on 0 or 255 when mixxx is started, should test this.
	// iv) In the future this function might be moved to hercules.cpp as it might prove useful to a Windows implementation as well (Windows MIDI signals a bit differently though).

//	qDebug("%s ENTER --> ev.value %i, m_iPitchOffset %i, m_iPitchPrevious %i, last calc: %i, next calc: %i",ControlSide.data(), ev_value, m_iPitchOffset, m_iPitchPrevious, (m_iPitchPrevious + m_iPitchOffset), (ev_value + m_iPitchOffset));
	if (m_iPitchOffset==-9999) {
		m_iPitchOffset = 127 - ev_value;
//		qDebug("%s PITCH OFFSET INIT/RESET ev_value %i, m_iPitchOffset %i",ControlSide.data(), ev_value, m_iPitchOffset);
	}

	if ((m_iPitchPrevious + m_iPitchOffset) == 255  && m_iPitchPrevious < ev_value) {
		m_iPitchOffset = (255 - ev_value);
//		qDebug("%s MAX + ROLLOVER",ControlSide.data());
	} else if (m_iPitchPrevious == 255 && ev_value == 0) {
		m_iPitchOffset = (255 + m_iPitchOffset);
//		qDebug("%s MAX",ControlSide.data());
	} else if (ev_value == 255 && m_iPitchPrevious == 0 && m_iPitchOffset >= 0) {
		m_iPitchOffset = (m_iPitchOffset - 255);
//		qDebug("%s ROLL DOWN",ControlSide.data());
	} else if (ev_value < m_iPitchPrevious && m_iPitchPrevious + m_iPitchOffset == 0) {
		m_iPitchOffset = (- ev_value);
//		qDebug("%s MIN ROLLDOWN #1",ControlSide.data());
	} 

	m_iPitchPrevious = ev_value;
//	qDebug("%s ADJUSTED m_iPitchOffset %i, m_iPitchPrevious %i, Resulting Pitch %5.3f", ControlSide.data(), m_iPitchOffset, m_iPitchPrevious, (((m_iPitchPrevious + m_iPitchOffset)-.5)/2.));
	return (((m_iPitchPrevious + m_iPitchOffset)-.5)/2.);
}

