/***************************************************************************
                          powermate.cpp  -  description
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

#include "powermate.h"
#include <linux/input.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "controlobject.h"
#include "controleventmidi.h"
#include "qapplication.h"
#include "midiobject.h"
#include "mathstuff.h"

#ifndef MSC_PULSELED
/* this may not have made its way into the kernel headers yet ... */
#define MSC_PULSELED 0x01
#endif

QValueList <int> PowerMate::openDevs;

PowerMate::PowerMate(ControlObject *_control)
{
    control = _control;
    fd = -1;
    id = -1;
    sendKnobEvent = false;
    fadeIn = false;
    magnitude=0;

    knob_integral = new int[KNOB_INTEGRAL_LEN];
    for (int i=0; i<KNOB_INTEGRAL_LEN; i++)
        knob_integral[i] = 0;

    requestLed = new QSemaphore(5);
}

PowerMate::~PowerMate()
{
    terminate();
    wait();
    closedev();
    delete [] knob_integral;
    delete requestLed;
}

bool PowerMate::opendev()
{
    fd = find(O_RDWR|O_NONBLOCK);
    if (fd>0)
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

void PowerMate::led()
{
    requestLed->tryAccess(1);
}

void PowerMate::run()
{
    struct input_event ibuffer[INPUT_BUFFER_SIZE];
    int r, events, i;

    timeval *waittime = new timeval;
    while (1)
    {
	r = read(fd, ibuffer, sizeof(struct input_event) * INPUT_BUFFER_SIZE);
        if(r > 0)
        {
            events = r / sizeof(struct input_event);
            for(i=0; i<events; i++)
                process_event(&ibuffer[i]);
        }

        //
        // Check if led queue is empty
        //

        // If last event was a knob event, send out zero value of knob
        if (sendKnobEvent)
            knob_event();

        // Check if we have to turn on led
        if (requestLed->available()==0)
        {
            (*requestLed)--;
            led_write(255, 0, 0, 0, 1);

            // Sleep
            waittime->tv_sec  = 0;
            waittime->tv_usec = 1;
            select(0,0,0,0,waittime);
	   
	    led_write(0, 0, 0, 0, 0);
	}
	else
	{
            // Sleep
            waittime->tv_sec  = 0;
            waittime->tv_usec = 50;
            select(0,0,0,0,waittime);
        }
    }
}
    
int PowerMate::find(int mode)
{
    for(int i=0; i<NUM_EVENT_DEVICES; i++)
    {
        if (openDevs.find(i)==openDevs.end())
        {
            int r = opendev(i, mode);
            if(r >= 0)
                return r;
        }
    }
    return -1;
}

int PowerMate::opendev(int _id, int mode)
{
    char devname[256];
    sprintf(devname, "/dev/input/event%d", _id);
    int fd = open(devname, mode);
    int i;
    char name[255];

    if(fd < 0)
        return -1;

    if(ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
    {
        close(fd);
        return -1;
    }

    // it's the correct device if the prefix matches what we expect it to be:
    for(i=0; i<NUM_VALID_PREFIXES; i++)
        if(!strncasecmp(name, valid_prefix[i], strlen(valid_prefix[i])))
        {
            id = _id;
            instno = openDevs.count();

            // Add id to list of open devices
            openDevs.append(id);
            
            return fd;
        }

    close(fd);
    return -1;
}

void PowerMate::closedev()
{
    if (fd>0)
    {
        close(fd);

        // Remove id from list
        QValueList<int>::iterator it = openDevs.find(id);
        if (it!=openDevs.end())
            openDevs.remove(it);
    }
    fd = -1;
    id = -1;
}        

void PowerMate::led_write(int static_brightness, int speed, int table, int asleep, int awake)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(struct input_event));

    static_brightness &= 0xFF;

    if(speed < 0)
        speed = 0;
    if(speed > 510)
        speed = 510;
    int direction = sign((float)knobval);
    if(table < 0)
        table = 0;
    if(table > 2)
        table = 2;
    asleep = !!asleep;
    awake = !!awake;

    ev.type = EV_MSC;
    ev.code = MSC_PULSELED;
    ev.value = static_brightness | (speed << 8) | (table << 17) | (asleep << 19) | (awake << 20);

    if(write(fd, &ev, sizeof(struct input_event)) != sizeof(struct input_event))
        qDebug("PowerMate: write(): %s", strerror(errno));
}

void PowerMate::process_event(struct input_event *ev)
{
    switch(ev->type)
    {
    case EV_REL:
        if(ev->code == REL_DIAL)
        {
            // Update knob variables
            knobval = ev->value;
            sendKnobEvent = true;
        }
        break;
    case EV_KEY:
        if(ev->code == BTN_0)
        {
            // Send event to GUI thread
            if (ev->value==1)
                QApplication::postEvent(control,new ControlEventMidi(NOTE_ON, POWERMATE_MIDI_CHANNEL, (char)(instno*2+POWERMATE_MIDI_BTN_CTRL),1));
            else
                QApplication::postEvent(control,new ControlEventMidi(NOTE_OFF, POWERMATE_MIDI_CHANNEL, (char)(instno*2+POWERMATE_MIDI_BTN_CTRL),1));

//            qDebug("PowerMate: Button was %s %i", ev->value? "pressed":"released",ev->value);
        }
        break;
    }
}

void PowerMate::knob_event()
{
    int direction = sign((float)knobval);
/*
    knobval *= 20*direction;

    // Find state
    if (!fadeIn && knobval!=0)
        fadeIn = true;
    else if (fadeIn && knobval==0)
        fadeIn = false;

    // Set new magnitude
    if (fadeIn)
    {
        if (magnitude < knobval)
            magnitude = min(magnitude+0.5,knobval);
    }
    else
    {
        if (magnitude > 0)
            magnitude = max(magnitude-0.5,0);
    }        
    
    // Post event
    //qDebug("knobval: %i, magnitude: %i, fadeIn: %i, midi ctrl: %i, instno: %i",knobval,magnitude,fadeIn,(instno*2),instno);
    QApplication::postEvent(control,new ControlEventMidi(CTRL_CHANGE, POWERMATE_MIDI_CHANNEL, (char)(instno*2+POWERMATE_MIDI_DIAL_CTRL),(char)((int)(magnitude*direction)+64)));

                                        
*/

    // Move everything one step backwards in integral buffer
    magnitude = 0;
    bool stop = true;
    for (int i=0; i<KNOB_INTEGRAL_LEN-1; i++)
    {
        knob_integral[i]=knob_integral[i+1];
        magnitude += knob_integral[i];
        if (knob_integral[i]!=0)
            stop = false;
    }
    knob_integral[KNOB_INTEGRAL_LEN-1] = knobval;
    magnitude += knob_integral[KNOB_INTEGRAL_LEN-1];

    // Range check
    if (magnitude>63)
        magnitude = 63;
    else if (magnitude<-64)
        magnitude = -64;

    // Post event
    //qDebug("knobval: %i, magnitude: %i, fadeIn: %i, midi ctrl: %i, instno: %i",knobval,magnitude,fadeIn,(instno*2),instno);
    QApplication::postEvent(control,new ControlEventMidi(CTRL_CHANGE, POWERMATE_MIDI_CHANNEL, (char)(instno*2+POWERMATE_MIDI_DIAL_CTRL),(char)((int)(magnitude)+64)));

    if (stop && magnitude==0)
        sendKnobEvent = false;

    // Reset knob value
    knobval = 0;
}


