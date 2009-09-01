/**
  * @file mididevicedummy.h
  * @author Albert Santoni alberts@mixxx.org
  * @date Sun Aug 9 2009
  * @brief Dummy MIDI backend
  *
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIDIDEVICEDUMMY_H
#define MIDIDEVICEDUMMY_H

#include <QtCore>
#include "mididevice.h"

/**
  *@author Albert Santoni
  */

/** A dummy implementation of MidiDevice */
class MidiDeviceDummy : public MidiDevice {
public:
    MidiDeviceDummy(MidiMapping* mapping=NULL) : MidiDevice(mapping) { setName("Dummy MIDI Device");};
    ~MidiDeviceDummy() {};
    int open() { return 0; };
    int close() { return 0; };
    void sendShortMsg(unsigned int word) {};
    void sendSysexMsg(unsigned char data[], unsigned int length) {};
protected:

};

#endif
