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

#ifndef MSC_PULSELED
/* this may not have made its way into the kernel headers yet ... */
#define MSC_PULSELED 0x01
#endif


PowerMate::PowerMate(ControlObject *_control)
{
    control = _control;
    id = -1;
    resetTimeCount = 0;
}

PowerMate::~PowerMate()
{
    terminate();
    wait();
    closedev();
}

bool PowerMate::opendev()
{
    id = find(O_RDWR|O_NONBLOCK);
    if (id>0)
    {
        // Start thread
        start();

        // Turn off led
        pulse_led(0,0,0,0,0);

        return true;
    }
    else
        return false;
}

void PowerMate::led(int)
{
}

void PowerMate::led_oneshot(int)
{
}

void PowerMate::run()
{
    struct input_event ibuffer[INPUT_BUFFER_SIZE];
    int r, events, i;

    timeval *waittime = new timeval;
    while (1)
    {
        r = read(id, ibuffer, sizeof(struct input_event) * INPUT_BUFFER_SIZE);
        if(r > 0)
        {
            events = r / sizeof(struct input_event);
            for(i=0; i<events; i++)
                process_event(&ibuffer[i]);
        }
        else
        {
            // Sleep
            waittime->tv_sec  = 0;
            waittime->tv_usec = 10;
            select(0,0,0,0,waittime);
        }    

        //
        // Check if led queue is empty
        //

        // If last event was a knob event, send out zero value of knob
        if (resetTimeCount>0)
        {
            resetTimeCount--;
            if (resetTimeCount==0)
                QApplication::postEvent(control,new ControlEventMidi(CTRL_CHANGE, POWERMATE_MIDI_CHANNEL, POWERMATE_MIDI_DIAL_CTRL,(char)64));
        }
    }
}
    
int PowerMate::find(int mode)
{
    char devname[256];
    int i, r;

    for(i=0; i<NUM_EVENT_DEVICES; i++)
    {
        qDebug("0");
        sprintf(devname, "/dev/input/event%d", i);
        r = opendev(devname, mode);
        if(r >= 0)
            return r;
    }

    return -1;
}

int PowerMate::opendev(const char *dev, int mode)
{
    int fd = open(dev, mode);
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
            return fd;

    close(fd);
    return -1;
}

void PowerMate::closedev()
{
    if (id>0)
        close(id);
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
    if(table < 0)
        table = 0;
    if(table > 2)
        table = 2;
    asleep = !!asleep;
    awake = !!awake;

    ev.type = EV_MSC;
    ev.code = MSC_PULSELED;
    ev.value = static_brightness | (speed << 8) | (table << 17) | (asleep << 19) | (awake << 20);

    if(write(id, &ev, sizeof(struct input_event)) != sizeof(struct input_event))
        qDebug("PowerMate: write(): %s", strerror(errno));
}

void PowerMate::process_event(struct input_event *ev)
{
    switch(ev->type)
    {
    case EV_REL:
        if(ev->code == REL_DIAL)
        {
            // Send event to GUI thread
            QApplication::postEvent(control,new ControlEventMidi(CTRL_CHANGE, POWERMATE_MIDI_CHANNEL, POWERMATE_MIDI_DIAL_CTRL,(char)(ev->value*10)+64));
            resetTimeCount = RESET_TIME;
//            qDebug("PowerMate: Button was rotated %d units", (int)ev->value);
        }
        break;
    case EV_KEY:
        if(ev->code == BTN_0)
        {
            // Send event to GUI thread
            if (ev->value==1)
                QApplication::postEvent(control,new ControlEventMidi(NOTE_ON, POWERMATE_MIDI_CHANNEL, POWERMATE_MIDI_BTN_CTRL,1));
            else
                QApplication::postEvent(control,new ControlEventMidi(NOTE_OFF, POWERMATE_MIDI_CHANNEL, POWERMATE_MIDI_BTN_CTRL,1));

//            qDebug("PowerMate: Button was %s %i", ev->value? "pressed":"released",ev->value);
        }
        break;
    }
}

