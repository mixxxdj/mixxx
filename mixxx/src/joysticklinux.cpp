/***************************************************************************
                          joysticklinux.cpp  -  description
                             -------------------
    begin                : Thu Jul 10 2003
    copyright            : (C) 2003 by Svein Magne Bang
    email                : sveinmb@stud.ntnu.no
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "joysticklinux.h"
#include "controlobject.h"
#include "controleventmidi.h"


JoystickLinux::JoystickLinux(ControlObject *pControl) : Joystick(pControl){
}

JoystickLinux::~JoystickLinux(){
}


int JoystickLinux::opendev(){

    char joydevice[50];  // device name
    int joystick_no;

    joystick_no=0;
  
    sprintf(joydevice, "/dev/js%i", joystick_no);

    // open joystick device
    if ((joystickDevice = open(joydevice, O_RDONLY)) < 0) {
      qDebug(joydevice);
      return 0;
    }

    ioctl(joystickDevice, JSIOCGAXES, &joystick.axes);
    ioctl(joystickDevice, JSIOCGBUTTONS, &joystick.buttons);
    ioctl(joystickDevice, JSIOCGNAME(255), joystick.name);

    qDebug("Joystick: Using device %s (%s), %i axes, %i buttons.\n", joydevice, joystick.name, joystick.axes, joystick.buttons);


    start();

    return 1;

}

void JoystickLinux::closedev(){
    close(joystickDevice);
}

void JoystickLinux::run(){

    while (1) {
        // read eventdata from joystick
        if (read(joystickDevice, &joystickEvent, sizeof(struct js_event)) != sizeof(struct js_event)) {
            qDebug("Joystick: error reading from joystick device");
            return;
        }

        // switch to right event
        switch(joystickEvent.type & ~JS_EVENT_INIT) {

            case JS_EVENT_BUTTON:
                // alternate between NOTE_ON and NOTE_OFF on each button event
                if (buttonvalue[joystickEvent.number] != 0){
                   QApplication::postEvent(m_pControl,new ControlEventMidi(NOTE_OFF, kiJoystickMidiChannel, joystickEvent.number,1));
                   buttonvalue[joystickEvent.number] = 0;
                } else {
                   QApplication::postEvent(m_pControl,new ControlEventMidi(NOTE_ON, kiJoystickMidiChannel, joystickEvent.number,1));
                   buttonvalue[joystickEvent.number] = 1;
                }                    
                break;
        
            case JS_EVENT_AXIS:
                // convert axis value into a short value
               value = (int) (((((double) joystickEvent.value) + SHRT_MAX) / USHRT_MAX)*127.0);
  
                // send midi data (only if value has changed since last event)
                if (axisvalue[joystickEvent.number] != value){
                    axisvalue[joystickEvent.number] = value;
                    QApplication::postEvent(m_pControl,new ControlEventMidi(CTRL_CHANGE, kiJoystickMidiChannel, joystickEvent.number+20, value));
                }
                break;
      
        }
    }
}

