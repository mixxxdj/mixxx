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
#include "rotary.h"
#include <linux/input.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "controlobject.h"
#include "mathstuff.h"

#ifndef MSC_PULSELED
  // this may not have made its way into the kernel headers yet ...
  #define MSC_PULSELED 0x01
#endif

QValueList <int> PowerMateLinux::sqlOpenDevs;

PowerMateLinux::PowerMateLinux() : PowerMate()
{
    m_iFd = -1;
    m_iId = -1;
    m_ctrlVuMeter = ControlObject::getControl(ConfigKey("[Channel2]","VuMeter"));
}

PowerMateLinux::~PowerMateLinux()
{
	//Turn off blue LED.
	led_write(0, 0, 0, 0, 0);
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
        if (kqPowermateValidPrefix[i]==rgcName)
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

void PowerMateLinux::getNextEvent()
{
/*
    // Apparently the PowerMate driver does not support select
    FD_ZERO(&fdset);
    FD_SET(m_iFd, &fdset);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int v = select(m_iFd+1, &fdset, 0, 0, &tv);
    if (v>0)
*/
    struct input_event ev;
    int iR = read(m_iFd, &ev, sizeof(struct input_event));
    static int iUnreadCount = 0;
    static double dSpeed = 0.0f;
    static int wait = 0;
    static int led_value = 0; //The brightness of the LED. 
    
    if (iR == sizeof(struct input_event))
    {
        switch(ev.type)
        {
        case EV_REL:
            if (ev.code == REL_DIAL)
            {
                int v = ev.value;
                
                //This next line was used in the original Powermate code... Long story short: It screws up everything.
                //double dValue = m_pRotary->filter((double)v);
                
                //qDebug("POWERMATE: v = %d, dValue = %f", v, dValue);
                
                //Scale the speed of the seeking/scratching/whatever based on how fast the Powermate is being turned.
                if (v >= 2 || v <= -2)
                	v *= 200;
                //if (v >= 3)
                //	v *= 20;
                //if (v >= 4)
                //	v *= 500;
                //Note: If those numbers look ridiculously, it's probably because they are.
                
                
		        if (m_pControlObjectButton != NULL)
		        {
		        	if (m_pControlObjectButton->get() == 0.0f) //Only process the rotary knob when we're paused.
		        	{
		        		dSpeed = (double)v;
		        			                
		                sendEvent(dSpeed, m_pControlObjectRotary);
                	}
                }
            }
            break;
        case EV_KEY:
            if(ev.code == BTN_0)
            {
                // Send event to GUI thread
                if (ev.value==1)
                {
                	m_pControlObjectRotary->queueFromThread(0.0f); //Disable the scratch/wheel/whatever
                    sendButtonEvent(true, m_pControlObjectButton); //Start playback
                }
                else
                	
                    sendButtonEvent(false, m_pControlObjectButton); //Stop playback
                    m_pControlObjectRotary->queueFromThread(0.0f); //Reset the scratch/wheel/whatever
            }
            break;
        /*default:
       		qDebug("POWERMATE :: [type=0x%04x code=0x%04x, value=%d]", ev.type, ev.code, (int)ev.value);
       	*/
        }
    }
    else
    {
        //qDebug("unread");
        iUnreadCount++;
        if (iUnreadCount > 5)
       	{
       		iUnreadCount = 0;
       		m_pControlObjectRotary->queueFromThread(0.0f);
       	} 	
       
        /*if (m_pControlObjectButton != NULL)
        {
       		sendEvent((double)0.0f, m_pControlObjectRotary);
        }*/
    }
            

    
    if (m_ctrlVuMeter != NULL)
    {
    	//qDebug("POWERMATE: m_ctrlVuMeter->get() is %f", m_ctrlVuMeter->get());
    	if (wait > 5) //Save on CPU a bit...
    	{
    		if (m_pControlObjectButton->get() == 0.0f) //If we're paused, pulse the
    												   //blue light on the Powermate slowly (at a constant rate)
    		{
				led_write(255, 255, 0, 0, 1);
    		}
    		else //If we're playing, pulse the blue light on the Powermate to the music :)
     		{
     			led_value = (int)round(((m_ctrlVuMeter->get() * 255)) - 80);
    		    
    		    if (led_value < 0)
					led_value = 0;
    			
    			led_write(led_value, 0, 0, 0, 0);
			}
    		
    		wait = 0;
    	}
    	else
    		wait++;
    }
    
    // Check if we have to turn on led
    //led();
    
    //I have absolutely no idea what this does, but if you call led(), this stuff will get called. ~asantoni
    //
    // Check if led queue is empty
    //
    if (m_qRequestLed.available()==0)
    {
    	//qDebug("POWERMATE: LED!!!!");
        m_qRequestLed--;
        led_write(255, 0, 0, 0, 1);

        msleep(5);

        //led_write(0, 0, 0, 0, 0);
    }
    else if (iR != sizeof(struct input_event))
        msleep(5);
}

/*	Useful PowerMate LED terminology:
	brightness:  LED-brightness (0-255)
	speed:       pulse-speed (0-510)
	mode (table?): pule-style (0,1,2)
	sleep:       pulse while host is off
	awake:       pulse while host is on
*/
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
