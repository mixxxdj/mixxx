/***************************************************************************
                          mouselinux.cpp  -  description
                             -------------------
    begin                : Wed Oct 6 2004
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

#include "mouselinux.h"
#include "powermatelinux.h"
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include "controlobject.h"
#include "mathstuff.h"
#include <qstringlist.h>

MouseLinux::MouseLinux() : Mouse()
{
    m_iFd = -1;
}

MouseLinux::~MouseLinux()
{
}

bool MouseLinux::opendev(QString name)
{
    qDebug("try open %s",name.latin1());
    for(int iId=0; iId<kiMouseNumDevices; iId++)
    {
        char rgcDevName[256];
        sprintf(rgcDevName, "/dev/input/event%d", iId);
        int iFd = open(rgcDevName, O_RDONLY|O_NONBLOCK);
        if(iFd >=0)
        {
            // Get name of device
            char rgcName[255];
            if(ioctl(iFd, EVIOCGNAME(sizeof(rgcName)), rgcName) >= 0)
            {
                // Check if rgcName matches name
                if (name==QString("%1 (%2)").arg(rgcName).arg(iId))
                {
                    m_iFd = iFd;


                    start();
                    return true;
                }
            }
        }
    }
    return false;
}

void MouseLinux::closedev()
{
    if (m_iFd>0)
        close(m_iFd);
    m_iFd = -1;
}

void MouseLinux::getNextEvent()
{
        struct input_event ev;

        FD_ZERO(&fdset);
        FD_SET(m_iFd, &fdset);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000;
        int r = pselect(m_iFd+1, &fdset, NULL, NULL, &ts, 0);
        if (r>0)
        {
            int iR = read(m_iFd, &ev, sizeof(struct input_event));
            if (iR == sizeof(struct input_event) && ev.type==EV_REL && ev.code==REL_X)
            {
                int v = ev.value;
                sendRotaryEvent((double)v);
            }
            else
                sendRotaryEvent(0.);
        }
        else
            sendRotaryEvent(0.);
}

QStringList MouseLinux::getDeviceList()
{
    QStringList sResult;

    sResult << "None";

    for(int iId=0; iId<kiMouseNumDevices; iId++)
    {
        char rgcDevName[256];
        sprintf(rgcDevName, "/dev/input/event%d", iId);
        int iFd = open(rgcDevName, O_RDONLY);
        int i;
        if(iFd >=0)
        {
            // Get name of device
            char rgcName[255];
            if(ioctl(iFd, EVIOCGNAME(sizeof(rgcName)), rgcName) >=0)
            {
                // Check that this is not a PowerMate...
                bool bPowerMate = false;
                for(i=0; i<kiPowermateNumValidPrefixes; i++)
                {
                    if(kqPowermateValidPrefix[i]==rgcName)
                        bPowerMate = true;
                }

                if (!bPowerMate)
                    sResult << QString("%1 (%2)").arg(rgcName).arg(iId);
            }
        }
    }

    return sResult;
}
