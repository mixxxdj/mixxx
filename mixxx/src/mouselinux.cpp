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
#include "rotary.h"
#include "powermatelinux.h"
#include <sys/time.h>
#include <QtDebug>
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
    m_bSending = false;
}

MouseLinux::~MouseLinux()
{
}

bool MouseLinux::opendev(QString name)
{
//    qDebug() << "try open" << name;
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

                    // QT 3.3:
                    //start(QThread::TimeCriticalPriority);

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
    // Select time to wait. If we just received data, we want to wait longer for it (100ms), if the
    // select call timed out last time, we don't want to wait so long time, because we want to have the
    // low-pass filter in Rotary reach zero fast, if movement has stopped (10ms)
    if (m_bSending)
        ts.tv_nsec = 100000000;
    else
        ts.tv_nsec = 10000000;

    int r = pselect(m_iFd+1, &fdset, NULL, NULL, &ts, 0);
//         qDebug() << "waited " << ts.tv_sec << "," << ts.tv_nsec;
    if (r>0)
    {
        m_bSending = true;
        int iR = read(m_iFd, &ev, sizeof(struct input_event));
        if (iR == sizeof(struct input_event) && ev.type==EV_REL && ev.code==REL_X)
        {
            int v = ev.value;

//                 qDebug() << "send value";

            double dValue = m_pRotary->filter((double)v);
            sendEvent(dValue, m_pControlObjectRotary);
        }
    }
    else
    {
//             qDebug() << "timeout";
        sendEvent(0., m_pControlObjectRotary);
        m_bSending = false;
    }
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
