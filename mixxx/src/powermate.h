/***************************************************************************
                          powermate.h  -  description
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

#ifndef POWERMATE_H
#define POWERMATE_H

#include <qthread.h>
#include <qsemaphore.h>
#include <qvaluelist.h>

/**
  * Linux code for handling the PowerMate. This is implemented as a separate thread.
  * The code is heavily inspired by the powermate-1.0 package.
  *
  *@author Tue & Ken Haste Andersen
  */

class ControlObject;

  
// Parameters used to interface the PowerMate with the MIDI code
const int POWERMATE_MIDI_CHANNEL=16;
const int POWERMATE_MIDI_DIAL_CTRL = 0;
const int POWERMATE_MIDI_BTN_CTRL = 1;

const int NUM_VALID_PREFIXES = 2;
static const char *valid_prefix[NUM_VALID_PREFIXES] =
{
    "Griffin PowerMate",
    "Griffin SoundKnob"
};
const int INPUT_BUFFER_SIZE = 32;
const int NUM_EVENT_DEVICES = 16;

const int KNOB_INTEGRAL_LEN = 40;

class PowerMate : public QThread
{
public: 
    PowerMate(ControlObject *_control);
    ~PowerMate();
    bool opendev();
    void led();
protected:
    void run();
private:
    int find(int mode);
    int opendev(int _id, int mode);
    void closedev();
    void led_write(int static_brightness, int speed, int table, int asleep, int awake);
    void process_event(struct input_event *ev);

    /** This method is called every 50ms during an active period of the knob. The method sends
      * knob events out, which has been interpolated and fitted to a certain function */
    void knob_event();
    /** File handle of current open /dev/input/event device */
    int fd;
    /** ID of event interface */
    int id;
    /** Instantiate number. Used in the calculation of MIDI controller id's */
    int instno;
    /** Pointer to ControlObject */
    ControlObject *control;
    /** Variable used to indicate weather a knob event should be sent or not */
    bool sendKnobEvent;
    /** Amplitude of knob */
    float magnitude;
    /** State of knob. true = fade in, false=fade out */
    bool fadeIn;
    /** Current value got from knob */
    int knobval;
    /** List of open devices */
    static QValueList <int> openDevs;
    /** Size of knob integral buffer */
    int *knob_integral;
    /** Semaphore used to control led */
    QSemaphore *requestLed;
};

#endif
