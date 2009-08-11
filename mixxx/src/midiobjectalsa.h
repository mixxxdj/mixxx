/***************************************************************************
                          midiobjectalsa.h  -  description
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

#ifndef MIDIOBJECTALSA_H
#define MIDIOBJECTALSA_H

#include <midiobject.h>
#include <alsa/asoundlib.h>

/**
  *@author Sean M. Pappalardo
  */

class MidiObjectALSA : public MidiObject  {
public: 
    MidiObjectALSA();
    ~MidiObjectALSA();
    int getDevicesList();
    void devOpen(QString device);
    void devClose();
//     int devOpen(QString device);
//     int devClose();
    void sendShortMsg(unsigned int word);
    void sendSysexMsg(unsigned char data[], unsigned int length);
protected:
    void run();
    int find_midi_devices_on_card(int card);
    int find_subdevice_info(snd_ctl_t *ctl, int card, int device);
    int is_input(snd_ctl_t *ctl, int card, int device, int sub);
    int is_output(snd_ctl_t *ctl, int card, int device, int sub);

    snd_rawmidi_t   *m_inHandle;
    snd_rawmidi_t   *m_outHandle;
    char            *m_buffer;
    QMap<QString, QString>  m_availableDevices;   //The name and ALSA device string of available MIDI devices
    // Try QMap<QString, QChar[]>
    int m_input;
    bool m_run;
};

#endif
