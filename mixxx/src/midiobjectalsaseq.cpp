/***************************************************************************
                          midiobjectalsaseq.cpp  -  description
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

#include "midiobjectalsaseq.h"

MidiObjectALSASeq::MidiObjectALSASeq() : MidiObject("")
{
    openDevice = "alsa-sequencer";

    devices.append(openDevice);
    int err;
    snd_seq_port_info_t *pinfo;

    err = snd_seq_open(&m_handle,  "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
    if (err != 0)
    {
      	qDebug("Open of midi failed: %s.", snd_strerror(err));
	qDebug("have you the snd-seq-midi kernel module loaded?");
	qDebug("If not, launch modprobe snd-seq-midi as root");
	return;
    }
    m_client = snd_seq_client_id(m_handle);
    snd_seq_set_client_name(m_handle, "Mixxx");

    m_queue = snd_seq_alloc_named_queue(m_handle, "Mixxx_queue");
    if (m_queue != 0)
    {
        qDebug("Creation of the midi queue failed.");
        return;
    }

    snd_seq_port_info_alloca(&pinfo);
    snd_seq_port_info_set_capability(pinfo, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE );
    snd_seq_port_info_set_type(pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION );
    snd_seq_port_info_set_midi_channels(pinfo, 16);
    snd_seq_port_info_set_timestamping(pinfo, 1);
    snd_seq_port_info_set_timestamp_real(pinfo, 1);
    snd_seq_port_info_set_timestamp_queue(pinfo, m_queue);
    snd_seq_port_info_set_name(pinfo, "input");
    m_input = snd_seq_create_port(m_handle, pinfo);
    if (m_input != 0)
    {
        qDebug("Creation of the input port failed" );
	return;
    }

    err = snd_seq_connect_from(m_handle, m_input, SND_SEQ_CLIENT_SYSTEM, SND_SEQ_PORT_SYSTEM_ANNOUNCE);
    if (err != 0)
    {
      	qDebug("snd_seq_connect_from failed" );
	return;
    }
    start();
}

MidiObjectALSASeq::~MidiObjectALSASeq()
{
    if (!snd_seq_free_queue(m_handle, m_queue))
      qDebug("snd_seq_free_queue failed" );
    if(!snd_seq_close(m_handle));
      qDebug("snd_seq_close failed" );
}

//no need to call devopen/devclose since, alsa midi sequencer will never change
void MidiObjectALSASeq::devOpen(QString)
{

}

void MidiObjectALSASeq::devClose()
{

}

void MidiObjectALSASeq::run()
{
    struct pollfd *pfds;
    int npfds;
    int rt;

    npfds = snd_seq_poll_descriptors_count(m_handle, POLLIN);
    pfds = (pollfd *)alloca(sizeof(*pfds) * npfds);
    snd_seq_poll_descriptors(m_handle, pfds, npfds, POLLIN);
    while(true)
    {
	rt = poll(pfds, npfds, 1000);
	if (rt < 0)
	  continue;
	do
	{
            snd_seq_event_t *ev;
            if (snd_seq_event_input(m_handle, &ev) >= 0 && ev)
            {
                char channel;
                char midicontrol;
                char midivalue;

                if (ev->type == SND_SEQ_EVENT_CONTROLLER)
                {
                   channel = ev->data.control.channel;
                   midicontrol = ev->data.control.param;
                   midivalue = ev->data.control.value;
                   send(CTRL_CHANGE, channel, midicontrol, midivalue);
                } else if (ev->type == SND_SEQ_EVENT_NOTEON)
                {
                   channel = ev->data.note.channel;
                   midicontrol = ev->data.note.note;
                   midivalue = ev->data.note.velocity;
                   send(NOTE_ON, channel, midicontrol, midivalue);
                } else if (ev->type == SND_SEQ_EVENT_NOTEOFF)
                {
                   channel = ev->data.note.channel;
                   midicontrol = ev->data.note.note;
                   midivalue = ev->data.note.velocity;
                   send(NOTE_OFF, channel, midicontrol, midivalue);
                } else if (ev->type == SND_SEQ_EVENT_NOTE)
                {
                  //what is a note event (a combinaison of a note on and a note off?)
                   qDebug("Midi Sequencer: NOTE!!" );
                }
                else
                {
                   qDebug("Midi Sequencer: unknown event" );
                }
            }
         }
         while (snd_seq_event_input_pending(m_handle, 0) > 0);
    }
}
