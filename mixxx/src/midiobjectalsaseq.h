/***************************************************************************
                          midiobjectalsaseq.h  -  description
                             -------------------
    begin                : Mon Sep 25 2006
    copyright            : (C) 2006 by Cedric GESTES
    email                : goctaf@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIDIOBJECTALSASEQ_H
#define MIDIOBJECTALSASEQ_H

#include <midiobject.h>
#include <alsa/asoundlib.h>

/**
  *@author Cedric GESTES
  */

class MidiObjectALSASeq : public MidiObject  {
public:
    MidiObjectALSASeq();
    ~MidiObjectALSASeq();
    int getClientPortsList(void);
    void devOpen(QString device);
    void devClose();
    void sendShortMsg(unsigned int word);
    void sendSysexMsg(unsigned char data[], unsigned int length);
protected:
    void run();

    snd_seq_t *m_handle;
    snd_seq_port_info_t *pinfo;
    QMultiMap<QString, int> sActivePorts;   //The name and client & port numbers we're currently connected to.
    int m_client;
    int m_input;
    int m_queue;
    bool m_run;
};

#endif
