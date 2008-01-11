/***************************************************************************
                          midiobjectalsaseq.cpp  -  description
                             -------------------
    begin                : Mon Sep 25 2006
    copyright            : (C) 2006 Cedric GESTES
                                           (C) 2007 Albert Santoni
    email                : goctaf@gmail.com
    note: parts of the code to poll ALSA's input client/ports is borrowed
          from qjackctl: http://qjackctl.sourceforge.net/
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QDebug>
#include "midiobjectalsaseq.h"

MidiObjectALSASeq::MidiObjectALSASeq(QString device) : MidiObject(device)
{
    int err;

    err = snd_seq_open(&m_handle,  "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
    if (err != 0)
    {
        qDebug("Open of midi failed: %s.", snd_strerror(err));
        qDebug("Do you have the snd-seq-midi kernel module loaded?");
        qDebug("If not, launch modprobe snd-seq-midi as root");
        return;
    }
    m_client = snd_seq_client_id(m_handle);
    snd_seq_set_client_name(m_handle, "Mixxx");

    //What the hell does creating a MIDI queue do?!
    //(Even if it fails, ALSA seq MIDI control will still work...)
    m_queue = snd_seq_alloc_named_queue(m_handle, "Mixxx_queue");
    if (m_queue != 0)
    {
        qDebug("Warning: Creation of the midi queue failed. %s", snd_strerror(m_queue));
        //return;
    }

    snd_seq_port_info_alloca(&pinfo);
    snd_seq_port_info_set_capability(pinfo, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE |
				     SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ );
    snd_seq_port_info_set_type(pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION );
    snd_seq_port_info_set_midi_channels(pinfo, 16);
    snd_seq_port_info_set_timestamping(pinfo, 1);
    snd_seq_port_info_set_timestamp_real(pinfo, 1);
    snd_seq_port_info_set_timestamp_queue(pinfo, m_queue);
    snd_seq_port_info_set_name(pinfo, "Mixxx");
    m_input = snd_seq_create_port(m_handle, pinfo);

    if (m_input != 0)
    {
        qDebug("Creation of the input port failed" );
        return;
    }

    // BJW Actually open the requested device (otherwise it won't be opened
    // until reconfigured via the Preferences dialog!)
    devOpen(device);

    start();
}

// Client:port refreshener (refreshener!) - Asks ALSA for a list of input clients/ports for MIDI
// and throws them in "devices", which is shown in the MIDI preferences dialog.
// Mostly borrowed from QJackCtl.
int MidiObjectALSASeq::getClientPortsList(void)
{
    if (m_handle == 0)
        return 0;

    int iDirtyCount = 0;

    //markClientPorts(0);
    unsigned int uiAlsaFlags;

    //Look for input ports only.
    uiAlsaFlags = SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ;

    //qDebug("****MIDI: Listing ALSA Sequencer clients:");

    //Clear the list of devices:
    while (!devices.empty())
        devices.pop_back();

    snd_seq_client_info_t * pClientInfo;
    snd_seq_port_info_t * pPortInfo;

    {
	// Use temporary variables to avoid misleading warnings
        // from the _alloca macros
	snd_seq_client_info_t ** ppCI = &pClientInfo;
	snd_seq_port_info_t ** ppPI = &pPortInfo;

	snd_seq_client_info_alloca(ppCI);
	snd_seq_port_info_alloca(ppPI);
    }
    snd_seq_client_info_set_client(pClientInfo, -1);

    while (snd_seq_query_next_client(m_handle, pClientInfo) >= 0) {
        int iAlsaClient = snd_seq_client_info_get_client(pClientInfo);
        if (iAlsaClient > 0) {
            snd_seq_port_info_set_client(pPortInfo, iAlsaClient);
            snd_seq_port_info_set_port(pPortInfo, -1);
            while (snd_seq_query_next_port(m_handle, pPortInfo) >= 0) {
                unsigned int uiPortCapability = snd_seq_port_info_get_capability(pPortInfo);
                if (((uiPortCapability & uiAlsaFlags) == uiAlsaFlags) &&
                    ((uiPortCapability & SND_SEQ_PORT_CAP_NO_EXPORT) == 0)) {
                    QString sClientName = QString::number(iAlsaClient) + ":";
                    sClientName += snd_seq_client_info_get_name(pClientInfo);
                    //qjackctlAlsaPort *pPort = 0;
                    int iAlsaPort = snd_seq_port_info_get_port(pPortInfo);

                    QString sPortName = QString::number(iAlsaPort) + ":";
                    sPortName += snd_seq_port_info_get_name(pPortInfo);
                    //qDebug(sPortName);
                    //qDebug("active port: " + sActivePortName);

                    if (sPortName == sActivePortName)                     //Make the port we're currently connected to be the first choice
                        //in the combobox in the MIDI preferences dialog.
                        devices.prepend(sPortName);
                    else
                        devices.append(sPortName);
                    iDirtyCount++;
                }
            }
        }
    }

    /* These cause a segfault, even if the pointers are good for some reason. (Yay ALSA documentation)
       if (pPortInfo)
            snd_seq_port_info_free(pPortInfo);
       if (pClientInfo)
            snd_seq_client_info_free(pClientInfo);
     */

    return iDirtyCount;
}

MidiObjectALSASeq::~MidiObjectALSASeq()
{
    if (!snd_seq_free_queue(m_handle, m_queue))
        qDebug("snd_seq_free_queue failed" );
    if (!snd_seq_close(m_handle))
	qDebug("snd_seq_close failed" );
}

//Connects the Mixxx ALSA seq device to a "client" picked from the combobox...
void MidiObjectALSASeq::devOpen(QString device)
{
    if (m_handle == 0)
        return;

    int iDirtyCount = 0;

    unsigned int uiAlsaFlags;

    //Look for input ports only.
    uiAlsaFlags = SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ;

    snd_seq_client_info_t * pClientInfo;
    snd_seq_port_info_t * pPortInfo;

    {
	// Use temporary variables to avoid misleading warnings
        // from the _alloca macros
	snd_seq_client_info_t ** ppCI = &pClientInfo;
	snd_seq_port_info_t ** ppPI = &pPortInfo;

	snd_seq_client_info_alloca(ppCI);
	snd_seq_port_info_alloca(ppPI);
    }
    snd_seq_client_info_set_client(pClientInfo, -1);

    while (snd_seq_query_next_client(m_handle, pClientInfo) >= 0) {
        int iAlsaClient = snd_seq_client_info_get_client(pClientInfo);
        if (iAlsaClient > 0) {
            snd_seq_port_info_set_client(pPortInfo, iAlsaClient);
            snd_seq_port_info_set_port(pPortInfo, -1);
            while (snd_seq_query_next_port(m_handle, pPortInfo) >= 0) {
                unsigned int uiPortCapability = snd_seq_port_info_get_capability(pPortInfo);
                if (((uiPortCapability & uiAlsaFlags) == uiAlsaFlags) &&
                    ((uiPortCapability & SND_SEQ_PORT_CAP_NO_EXPORT) == 0)) {
                    QString sClientName = QString::number(iAlsaClient) + ":";
                    sClientName += snd_seq_client_info_get_name(pClientInfo);
                    int iAlsaPort = snd_seq_port_info_get_port(pPortInfo);

                    QString sPortName = QString::number(iAlsaPort) + ":";
                    sPortName += snd_seq_port_info_get_name(pPortInfo);

                    if (sPortName == device)
                    {
                        //qDebug("Connecting " + sPortName + " to Mixxx");

                        snd_seq_connect_from(m_handle, m_input, iAlsaClient, iAlsaPort);
                        snd_seq_connect_to(m_handle, m_input, iAlsaClient, iAlsaPort);
                        sActivePortName = sPortName;
                    }
                    else {
                        //Disconnect Mixxx from any other ports (might be annoying, but let's us be safe.)
                        snd_seq_disconnect_from(m_handle, snd_seq_port_info_get_port(pPortInfo), iAlsaClient, iAlsaPort);
                        snd_seq_disconnect_to(m_handle, snd_seq_port_info_get_port(pPortInfo), iAlsaClient, iAlsaPort);
		    }

                    iDirtyCount++;
                }
            }
        }
    }

    /* These cause a segfault, even if the pointers are good for some reason. (Yay ALSA documentation)
       if (pPortInfo)
            snd_seq_port_info_free(pPortInfo);
       if (pClientInfo)
            snd_seq_client_info_free(pClientInfo);
     */

    //Update the list of clients for the combobox (sticks the new active device at the top of the list
    //so it's obvious that it's currently selected.)
    getClientPortsList();

    return;

}

void MidiObjectALSASeq::devClose()
{

}

void MidiObjectALSASeq::run()
{
    struct pollfd * pfds;
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
            snd_seq_event_t * ev;
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
                } else if (ev->type == SND_SEQ_EVENT_PITCHBEND)
                {
                    channel = ev->data.control.channel;
                    // BJW: Uniquely among the MIDI backends, ALSA SEQ "cooks" the pitchbend event
                    // into a number between -8192 and +8191. Here we convert it back to how the raw MIDI
                    // message would look (using the ALSA function pitchbend_decode()), because that's what
                    // the other backends supply. It then gets cooked again in MidiObject::send().
                    // qDebug("ALSA SEQ PITCH BEND: Cooked value: %d", ev->data.control.value);
                    int temp = ev->data.control.value + 8192;
                    midicontrol = temp & 0x7f;
                    midivalue = (temp >> 7) & 0x7f;
                    // qDebug("`-- Decooked to %d %d", midicontrol, midivalue);
                    send(PITCH_WHEEL, channel, midicontrol, midivalue);
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

void MidiObjectALSASeq::sendShortMsg(unsigned int word) {
    snd_seq_event_t ev;
    int byte1, byte2, byte3;

    // Safely retrieve the message sequence from the input
    byte1 = word & 0xff;
    byte2 = (word>>8) & 0xff;
    byte3 = (word>>16) & 0xff;
    // qDebug("MIDI message send via alsa seq: %02x %02x %02x", byte1, byte2, byte3);

    // Initialize the event structure
    snd_seq_ev_set_direct(&ev);
    snd_seq_ev_set_source(&ev, m_input);

    // Send to all subscribers
    snd_seq_ev_set_dest(&ev, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);

    // Decide which event type to choose
    switch ((byte1 & 0xf0)) {
    case 0x90:
        snd_seq_ev_set_noteon(&ev, byte1&0xf, byte2, byte3);
	break;
    case 0xb0:
	snd_seq_ev_set_controller(&ev, byte1&0xf, byte2, byte3);
	break;
    }
    snd_seq_event_output_direct(m_handle, &ev);
}
