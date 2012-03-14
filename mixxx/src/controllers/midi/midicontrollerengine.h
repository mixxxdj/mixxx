/**
* @file midicontrollerengine.h
* @author Sean M. Pappalardo spappalardo@mixxx.org
* @date Sat Mar 13 2012
* @brief MIDI controller extensions to the Controller engine
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef MIDICONTROLLERENGINE_H
#define MIDICONTROLLERENGINE_H

#include "controllers/controllerengine.h"

class MidiController;

class MidiControllerEngine : public ControllerEngine {

    public:
        MidiControllerEngine(MidiController* controller);
        ~MidiControllerEngine();

        // Execute a particular function with a data string (e.g. a device ID)
        bool execute(QString function, char status, char control, char value,
                     QString group);
};

#endif
