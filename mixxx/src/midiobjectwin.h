/***************************************************************************
                          midiobjectwin.h  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
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

#ifndef MIDIOBJECTWIN_H
#define MIDIOBJECTWIN_H

#include "midiobject.h"
#include "windows.h"
#include <QtCore>

/**
  *@author Tue & Ken Haste Andersen
  */

class MidiObjectWin : public MidiObject  {
public:
    MidiObjectWin();
    ~MidiObjectWin();
    void devOpen(QString device);
    void devClose();
    void handleMidi(char channel, char midicontrol, char midivalue);
    void sendShortMsg(unsigned int word);
    void sendSysexMsg(unsigned char data[], unsigned int length);
    virtual void updateDeviceList();

    QMap<QString, HMIDIIN> handles;
	QMap<QString, HMIDIOUT> outHandles;

protected:
    void run();
    void stop();
};

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

#endif
