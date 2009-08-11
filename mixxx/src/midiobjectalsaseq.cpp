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

MidiObjectALSASeq::MidiObjectALSASeq() : MidiObject()
{
    m_handle = NULL;
    pinfo = NULL;
    m_client = 0;
    m_input = 0;
    m_queue = 0;
    int err;

    err = snd_seq_open(&m_handle,  "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
    if (err != 0)
    {
        qDebug() << "Opening ALSA MIDI sequencer failed: " << snd_strerror(err) << ".";
        qDebug() << "Do you have the snd-seq-midi kernel module loaded?";
        qDebug() << "If not, launch modprobe snd-seq-midi as root";
        return;
    }
    m_client = snd_seq_client_id(m_handle);
    snd_seq_set_client_name(m_handle, "Mixxx");

    //What the hell does creating a MIDI queue do?!
    //(Even if it fails, ALSA seq MIDI control will still work...)
    //
    // Doesn't look like it's actually used, since it is never started and the send events use _direct
    //  See http://www.alsa-project.org/alsa-doc/alsa-lib/seq.html  - Sean
    m_queue = snd_seq_alloc_named_queue(m_handle, "Mixxx_queue");
    if (m_queue != 0)
    {
        qDebug() << "Warning: Creation of the midi queue failed. " << snd_strerror(m_queue);
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

    snd_seq_start_queue(m_handle, m_queue, NULL);

    if (m_input != 0)
    {
        qDebug() << "Creation of the input port failed";
        return;
    }

    getClientPortsList();

    // BJW Actually open the requested device (otherwise it won't be opened
    // until reconfigured via the Preferences dialog!)
    //devOpen(device);

    m_run = true;
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

    //qDebug() << "****MIDI: Listing ALSA Sequencer clients:";

    //Clear the list of devices:
    devices.clear();

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
                    //QString sClientName = QString::number(iAlsaClient) + ":";
                    //sClientName += snd_seq_client_info_get_name(pClientInfo);
                    //qjackctlAlsaPort *pPort = 0;
                    //int iAlsaPort = snd_seq_port_info_get_port(pPortInfo);

                    QString sPortName /*= QString::number(iAlsaPort) + ":";
                    sPortName */= snd_seq_port_info_get_name(pPortInfo);
                    //qDebug() << sPortName;
                    //qDebug() << "active port: " << sActivePortName;

                    //Blacklist the "Midi Through" and "Mixxx" output devices
                    //in order to avoid MIDI feedback loops (say, so the MIDI
                    //LED handlers don't fire MIDI messages back to Mixxx
                    //itself).
                    if (!sPortName.toLower().contains("midi through") &&
                        !sPortName.toLower().contains("mixxx"))
                    {
                        //if (sPortName == sActivePortName) //Make the port we're currently connected to be the first choice
                                                          //in the combobox in the MIDI preferences dialog.
                        //    devices.prepend(sPortName);
                        //else
                            devices.append(sPortName);
                        iDirtyCount++;
                    }
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
//     qDebug() << "In ~MidiObjectALSASeq()";
    m_run = false;
    wait();
    
    shutdown(); // From parent MidiObject
    devClose();
    
    int err;
    err = snd_seq_free_queue(m_handle, m_queue);
    if (err != 0) qDebug() << "Freeing MIDI queue failed: " << snd_strerror(err) << ".";
    err = snd_seq_close(m_handle);
    if (err != 0) qDebug() << "Closing ALSA MIDI sequencer failed: " << snd_strerror(err) << ".";
}

//Connects the Mixxx ALSA seq device to a "client" picked from the combobox...
void MidiObjectALSASeq::devOpen(QString device)
{
    if (m_handle == 0)
        return;

    int iDirtyCount = 0;

    unsigned int uiAlsaFlags;
    m_deviceName = device;

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
            
            //Look for input ports only.
            uiAlsaFlags = SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ;
            while (snd_seq_query_next_port(m_handle, pPortInfo) >= 0) {
                unsigned int uiPortCapability = snd_seq_port_info_get_capability(pPortInfo);
                if (((uiPortCapability & uiAlsaFlags) == uiAlsaFlags) &&
                    ((uiPortCapability & SND_SEQ_PORT_CAP_NO_EXPORT) == 0)) {
                    QString sClientName = QString::number(iAlsaClient) + ":";
                    sClientName += snd_seq_client_info_get_name(pClientInfo);
                    int iAlsaPort = snd_seq_port_info_get_port(pPortInfo);

                    QString sPortName = /*QString::number(iAlsaPort) + ":";
                    sPortName +=*/ snd_seq_port_info_get_name(pPortInfo);

                    if (sPortName == device)
                    {
                        //qDebug() << "Connecting input " + sPortName + " to Mixxx";
                        snd_seq_connect_from(m_handle, m_input, iAlsaClient, iAlsaPort);
//                         sActivePortNames.append(sPortName);
                        sActivePorts.insert(sPortName+"in", iAlsaPort);
                        sActivePorts.insert(sPortName+"in", iAlsaClient);
                    }
                    else {
                        //Disconnect Mixxx from any other input ports (might be annoying, but lets us be safe.)
                        snd_seq_disconnect_from(m_handle, snd_seq_port_info_get_port(pPortInfo), iAlsaClient, iAlsaPort);
                    }
                }
            }
            
            // Look for output port since it may be a different number
            snd_seq_port_info_set_port(pPortInfo, -1);
            uiAlsaFlags = SND_SEQ_PORT_CAP_WRITE  | SND_SEQ_PORT_CAP_SUBS_WRITE;
            while (snd_seq_query_next_port(m_handle, pPortInfo) >= 0) {
                unsigned int uiPortCapability = snd_seq_port_info_get_capability(pPortInfo);
                if (((uiPortCapability & uiAlsaFlags) == uiAlsaFlags) &&
                    ((uiPortCapability & SND_SEQ_PORT_CAP_NO_EXPORT) == 0)) {
                    QString sClientName = QString::number(iAlsaClient) + ":";
                    sClientName += snd_seq_client_info_get_name(pClientInfo);
                    int iAlsaPort = snd_seq_port_info_get_port(pPortInfo);

                    QString sPortName = /*QString::number(iAlsaPort) + ":";
                    sPortName +=*/ snd_seq_port_info_get_name(pPortInfo);

                    if (sPortName == device)
                    {
                        //qDebug() << "Connecting output " + sPortName + " to Mixxx";
                        // Output port may be different
                        snd_seq_connect_to(m_handle, m_input, iAlsaClient, iAlsaPort);
                        sActivePorts.insert(sPortName+"out", iAlsaPort);
                        sActivePorts.insert(sPortName+"out", iAlsaClient);
                    }
                    else {
                        //Disconnect Mixxx from any other output ports (might be annoying, but lets us be safe.)
                        snd_seq_disconnect_to(m_handle, snd_seq_port_info_get_port(pPortInfo), iAlsaClient, iAlsaPort);
                    }
                }
            }
            iDirtyCount++;
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
    QList<int> values;
    if (sActivePorts.contains(m_deviceName+"in")) {
        values = sActivePorts.values(m_deviceName+"in");
        qDebug() << "Disconnecting" << m_deviceName << "input on" << values;
        snd_seq_disconnect_from(m_handle, m_input, values.at(0), values.at(1));
        sActivePorts.remove(m_deviceName+"in");
    }
    
    if (sActivePorts.contains(m_deviceName+"out")) {
        values = sActivePorts.values(m_deviceName+"out");
        qDebug() << "Disconnecting" << m_deviceName << "output on" << values;
        snd_seq_disconnect_to(m_handle, m_input, values.at(0), values.at(1));
        sActivePorts.remove(m_deviceName+"out");
    }
}

void MidiObjectALSASeq::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("MidiObjectALSASeq %1").arg(++id));

#ifdef __MIDISCRIPT__
    MidiObject::run();
#endif

    struct pollfd * pfds;
    int npfds;
    int rt;

    npfds = snd_seq_poll_descriptors_count(m_handle, POLLIN);
    pfds = (pollfd *)alloca(sizeof(*pfds) * npfds);
    snd_seq_poll_descriptors(m_handle, pfds, npfds, POLLIN);
//     while(true)
    while(m_run)
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
                snd_seq_port_info_t * portInfo; //Info container for midi endpoint
                snd_seq_port_info_alloca(&portInfo);

                //Get info about sending midi endpoint
                snd_seq_get_any_port_info(m_handle, ev->source.client, ev->source.port, portInfo);

                if (ev->type == SND_SEQ_EVENT_CONTROLLER)
                {
                    channel = ev->data.control.channel;
                    midicontrol = ev->data.control.param;
                    midivalue = ev->data.control.value;
                    receive(MIDI_STATUS_CC, channel, midicontrol, midivalue);
                } else if (ev->type == SND_SEQ_EVENT_NOTEON)
                {
                    channel = ev->data.note.channel;
                    midicontrol = ev->data.note.note;
                    midivalue = ev->data.note.velocity;
                    receive(MIDI_STATUS_NOTE_ON, channel, midicontrol, midivalue);
                } else if (ev->type == SND_SEQ_EVENT_NOTEOFF)
                {
                    channel = ev->data.note.channel;
                    midicontrol = ev->data.note.note;
                    midivalue = ev->data.note.velocity;
                    receive(MIDI_STATUS_NOTE_OFF, channel, midicontrol, midivalue);
                } else if (ev->type == SND_SEQ_EVENT_PITCHBEND)
                {
                    channel = ev->data.control.channel;
                    // BJW: Uniquely among the MIDI backends, ALSA SEQ "cooks" the pitchbend event
                    // into a number between -8192 and +8191. Here we convert it back to how the raw MIDI
                    // message would look (using the ALSA function pitchbend_decode()), because that's what
                    // the other backends supply. It then gets cooked again in MidiObject::receive().
                    // qDebug() << "ALSA SEQ PITCH BEND: Cooked value: " << ev->data.control.value;
                    int temp = ev->data.control.value + 8192;
                    midicontrol = temp & 0x7f;
                    midivalue = (temp >> 7) & 0x7f;
                    // qDebug() << "`-- Decooked to " << midicontrol << " " << midivalue;
                    receive(MIDI_STATUS_PITCH_BEND, channel, midicontrol, midivalue);
                } else if (ev->type == SND_SEQ_EVENT_NOTE)
                {
                    //what is a note event (a combinaison of a note on and a note off?)
                    qDebug() << "Midi Sequencer: NOTE!!";
                }
                else
                {
                    //qDebug() << "Midi Sequencer: unknown event";
                    //You don't want this qDebug() enabled in a release version because
                    //if Joe DJ has some MIDI device plugged in that spits out MIDI clock
                    //signals, Mixxx will flood with these messages = hammer CPU.
                }
            }
        }
        while (m_run && (snd_seq_event_input_pending(m_handle, 0) > 0));
    }
}

void MidiObjectALSASeq::sendShortMsg(unsigned int word) {
    snd_seq_event_t ev;
    int byte1, byte2, byte3;
    
    // Safely retrieve the message sequence from the input
    byte1 = word & 0xff;
    byte2 = (word>>8) & 0xff;
    byte3 = (word>>16) & 0xff;
//     qDebug() << QString("MIDI message sent -- byte1: %1, byte2: %2, byte3: %3")
//         .arg(QString::number(byte1, 16).toUpper())
//         .arg(QString::number(byte2, 16).toUpper())
//         .arg(QString::number(byte3, 16).toUpper());

    // Initialize the event structure
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, m_input);

    // For using real-time mode
    snd_seq_real_time time;
    time.tv_sec = 0;
    time.tv_nsec = 1;
    snd_seq_ev_schedule_real(&ev, m_queue, 1, &time);

    // For using tick mode
    //static int tick = 0;
    //snd_seq_ev_schedule_tick(&ev, m_queue, 1, tick++);

    // For using direct mode
    //snd_seq_ev_set_direct(&ev);
    
    // Send to all subscribers
    //snd_seq_ev_set_dest(&ev, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);
    snd_seq_ev_set_subs(&ev);

    // Decide which event type to choose
    switch ((byte1 & 0xf0)) {
    case 0x80:  // Note off
        snd_seq_ev_set_noteoff(&ev, byte1&0xf, byte2, byte3);
        break;
    case 0x90:  // Note on
        snd_seq_ev_set_noteon(&ev, byte1&0xf, byte2, byte3);
        break;
    case 0xb0:  // Control Change
        snd_seq_ev_set_controller(&ev, byte1&0xf, byte2, byte3);
        break;
    default:
        qDebug() << QString("Unhandled status byte %1 in sendShortMsg").arg(QString::number(byte1, 16).toUpper());
        return;
    }

    // For direct output
    //result = snd_seq_event_output_direct(m_handle, &ev);

    // For scheduled output
    int result = snd_seq_event_output(m_handle, &ev);
        
    if (result < 0) {
        qDebug() << "sendShortMsg: event_output failed! -- message:"
                 << word << "result:" << result;
    }
        
    result = snd_seq_drain_output(m_handle);

    if (result > 0) {
        qDebug() << "sendShortMsg: drain_output failed: " << result;
    }

    // result = snd_seq_sync_output_queue(m_handle);
    // if (result != 0) {
    //     qDebug() << "Sync output failed: " << result;
    // }

}

// The sysex data must already contain the start byte 0xf0 and the end byte 0xf7.
void MidiObjectALSASeq::sendSysexMsg(unsigned char data[], unsigned int length) {
    snd_seq_event_t ev;

    // Initialize the event structure
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, m_input);

    // For scheduled output
    // snd_seq_real_time time;
    // time.tv_sec = 0;
    // time.tv_nsec = 1;
    // snd_seq_ev_schedule_real(&ev, m_queue, 1, &time);

    // For direct output
    snd_seq_ev_set_direct(&ev);

    // Send to all subscribers
    //snd_seq_ev_set_dest(&ev, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);
    snd_seq_ev_set_subs(&ev);
    
    // Do it
    snd_seq_ev_set_sysex(&ev,length,data);
    
    // For direct output
    snd_seq_event_output_direct(m_handle, &ev);
    
    // For scheduled output
    // int result = snd_seq_event_output(m_handle, &ev);
        
    // if (result < 0) {
    //     qDebug() << "sendSysexMsg: event_output failed! -- message len"
    //              << length << "result:" << result;
    // }
    
    // result = snd_seq_drain_output(m_handle);
    
    // if (result > 0) {
    //     qDebug() << "sendShortMsg: drain_output failed: " << result;
    // }    
}
