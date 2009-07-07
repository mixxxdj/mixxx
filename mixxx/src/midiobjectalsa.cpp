/***************************************************************************
                          midiobjectalsa.cpp  -  description
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

#include <QDebug>
#include "midiobjectalsa.h"

MidiObjectALSA::MidiObjectALSA() : MidiObject()
{
    m_inHandle = NULL;
    m_input = 0;
    
    getDevicesList();
}

MidiObjectALSA::~MidiObjectALSA()
{
//     qDebug() << "In ~MidiObjectALSA()";
    m_run = false;
    wait();
    
    shutdown(); // From parent MidiObject
    
    devClose();
}

// int MidiObjectALSA::devOpen(QString device)  // for when Albert wants to implement success checks
void MidiObjectALSA::devOpen(QString device)
{
    // Open RawMIDI device for input
    m_deviceName = device;
    
    qDebug() << "Opening ALSA device" << m_availableDevices.value(device) << device;
    
    int status;
    int mode = SND_RAWMIDI_NONBLOCK;    //SND_RAWMIDI_NONBLOCK, SND_RAWMIDI_APPEND, SND_RAWMIDI_SYNC
    if ((status = snd_rawmidi_open(&m_inHandle, &m_outHandle, m_availableDevices.value(device), mode)) < 0) {
        qDebug() << "Opening ALSA RawMIDI device failed: " << snd_strerror(status) << ".";
//         return -1;
        return;
    }
    else
    {
        // Allocate buffer
        m_buffer = new char[4096];
        if (m_buffer == 0)
        {
            qDebug() << "Midi: Error allocating buffer";
//             return -1;
            return;
        }

        // Set number of bytes received, before snd_rawmidi_read is woken up.
//         snd_rawmidi_params_t params;
//         params.channel = SND_RAWMIDI_CHANNEL_INPUT;
//         params.size    = 4096;
//         params.min     = 1;
//         err = snd_rawmidi_channel_params(m_inHandle,&params);
        // Start the midi thread:
        m_run = true;
        start();
//         return 0;
    }
}

// int MidiObjectALSA::devClose()
void MidiObjectALSA::devClose()
{
    // Stop the midi thread:
    m_run = false;
    wait();

    // Close device
    int err = snd_rawmidi_close(m_inHandle);
    if (err != 0) qDebug() << "Closing MIDI input device failed: " << snd_strerror(err) << ".";
    m_inHandle = NULL;  // snd_rawmidi_close() does not clear invalid pointer

    err = snd_rawmidi_close(m_outHandle);
    if (err != 0) qDebug() << "Closing MIDI output device failed: " << snd_strerror(err) << ".";
    m_outHandle = NULL; // snd_rawmidi_close() does not clear invalid pointer

    // Deallocate buffer
    delete [] m_buffer;
    
//     return err;
}

void MidiObjectALSA::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("MidiObjectALSASeq %1").arg(++id));

#ifdef __MIDISCRIPT__
    MidiObject::run();
#endif
    int status;
    char midiStatus;
    char midiControl;
    char midiValue;
    char buffer[1];        // Storage for input buffer received
    int i = 0;
    
//     qDebug() << "Thread Started";
    while(m_run) {
        status = 0;
        while (status != -EAGAIN) {
            status = snd_rawmidi_read(m_inHandle, buffer, 1);
            if ((status < 0) && (status != -EBUSY) && (status != -EAGAIN)) {
                qWarning() << "Problem reading MIDI input:" << snd_strerror(status);
            } else if (status >= 0) {
                i++;
                if ((unsigned char)buffer[0] >= 0x80) {
                    midiStatus = (unsigned char)buffer[0];
                } else {
                    if (i==2) midiControl = (unsigned char)buffer[0];
                    if (i==3) {
                        midiValue = (unsigned char)buffer[0];
                        receive((MidiStatusByte)(midiStatus & 0xF0), midiStatus & 0x0F, midiControl, midiValue);
                        i=0;
                    }
                }
            }
        }

/*
        do
        {
            int no = snd_rawmidi_read(m_inHandle,&m_buffer[0],1);
            if (no != 1)
                qDebug() << "Warning: midiobject recieved " << no << " bytes.";
        } while (m_run && ((m_buffer[0] & 128) != 128));

        if (m_run) {
            // and then get the following 2 bytes:
            char status;
            char midicontrol;
            char midivalue;
            for (int i=1; i<3; i++)
            {
                int no = snd_rawmidi_read(m_inHandle,&m_buffer[i],1);
                if (no != 1)
                    qDebug() << "Warning: midiobject recieved " << no << " bytes.";
            }
            status = m_buffer[0];
            midicontrol = m_buffer[1];
            midivalue = m_buffer[2];
            
            MidiStatusByte midiStatus;
            switch ((status & 0xF0)) {
                case 0x80:  // Note off
                    midiStatus = MIDI_STATUS_NOTE_OFF;
                    break;
                case 0x90:  // Note on
                    midiStatus = MIDI_STATUS_NOTE_ON;
                    break;
                case 0xb0:  // Control Change
                    midiStatus = MIDI_STATUS_CC;
                    break;
                case 0xe0:  // Pitch bend
                    midiStatus = MIDI_STATUS_PITCH_BEND;
                    break;
            }
    
            receive(midiStatus, status & 0x0F, midicontrol, midivalue);
        }
        */
    }
//     qDebug() << "Ending thread";
}

void MidiObjectALSA::sendShortMsg(unsigned int word)
{
    int status;
    unsigned char data[2];
//     int byte1, byte2, byte3;

    // Safely retrieve the message sequence from the input
    data[0] = word & 0xff;
    data[1] = (word>>8) & 0xff;
    data[2] = (word>>16) & 0xff;
    
    if ((status = snd_rawmidi_write(m_outHandle, data, 3)) < 0)
        qWarning() << "Problem writing to MIDI output:" << snd_strerror(status);


//     qDebug() << QString("MIDI message sent -- byte1: %1, byte2: %2, byte3: %3")
//         .arg(QString::number(byte1, 16).toUpper())
//         .arg(QString::number(byte2, 16).toUpper())
//         .arg(QString::number(byte3, 16).toUpper());
}

void MidiObjectALSA::sendSysexMsg(unsigned char data[], unsigned int length)
{
    int status;
    if ((status = snd_rawmidi_write(m_outHandle, data, length)) < 0)
        qWarning() << "Problem writing to MIDI output:" << snd_strerror(status);
}

// RawMIDI device list refresher - Asks ALSA for a list of MIDI input devices
// and throws them in "devices", which is shown in the MIDI preferences dialog.
// Mostly lifted from http://ccrma.stanford.edu/~craig/articles/linuxmidi/alsa-1.0/alsarawportlist.c
int MidiObjectALSA::getDevicesList()
{
    int status;
    int card = -1;  // use -1 to prime the pump of iterating through card list
    
    if ((status = snd_card_next(&card)) < 0) {
        qWarning() << "Cannot determine number of sound cards:" << snd_strerror(status);
        return 0;
    }
    if (card < 0) {
        qWarning() << "No sound cards found";
        return 0;
    }
    
    int foundDevices = 0;
    while (card >= 0) {
        foundDevices = find_midi_devices_on_card(card);
        if ((status = snd_card_next(&card)) < 0) {
            qWarning() << "Cannot determine number of sound cards:" << snd_strerror(status);
            break;
        }
    }
    return foundDevices;
}

//////////////////////////////
//
// find_midi_devices_on_card -- For a particular "card" look at all
//   of the "devices/subdevices" on it and get information about it
//   if it can handle MIDI input and/or output.
//
int MidiObjectALSA::find_midi_devices_on_card(int card)    // Might need to put this under the class
{
    snd_ctl_t *ctl;
    char name[32];
    int device = -1;
    int status;
    int foundDevices = 0;
    
    sprintf(name, "hw:%d", card);
//     qDebug() << name;
    if ((status = snd_ctl_open(&ctl, name, 0)) < 0) {
        qWarning() << "Cannot open control for card" << card << snd_strerror(status);
        return foundDevices;
    }
    do {
        status = snd_ctl_rawmidi_next_device(ctl, &device);
        if (status < 0) {
            qWarning() << "Cannot determine number of devices:" << snd_strerror(status);
            break;
        }
        if (device >= 0) {
            foundDevices = find_subdevice_info(ctl, card, device);
        }
    } while (device >= 0);
    snd_ctl_close(ctl);
    return foundDevices;
}

// find_subdevice_info -- Get information about a subdevice
//   of a device of a card if it can handle MIDI input and/or output.
//
int MidiObjectALSA::find_subdevice_info(snd_ctl_t *ctl, int card, int device) {
    snd_rawmidi_info_t *info;
    const char *name;
    const char *sub_name;
    int subs, subs_in, subs_out;
    int sub, in, out;
    int status;
    int foundDevices = 0;
    
    snd_rawmidi_info_alloca(&info);
    snd_rawmidi_info_set_device(info, device);
    
    snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
    snd_ctl_rawmidi_info(ctl, info);
    subs_in = snd_rawmidi_info_get_subdevices_count(info);
    snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
    snd_ctl_rawmidi_info(ctl, info);
    subs_out = snd_rawmidi_info_get_subdevices_count(info);
    subs = subs_in > subs_out ? subs_in : subs_out;
    
    sub = 0;
    in = out = 0;
    if ((status = is_output(ctl, card, device, sub)) < 0) {
        qWarning() << QString("Cannot get RawMIDI information %d:%d: %s")
            .arg(card).arg(device).arg(snd_strerror(status));
        return foundDevices;
    } else if (status)
        out = 1;
    
    if (status == 0) {
        if ((status = is_input(ctl, card, device, sub)) < 0) {
            qWarning() << QString("Cannot get RawMIDI information %d:%d: %s")
                .arg(card).arg(device).arg(snd_strerror(status));
            return foundDevices;
        }
    } else if (status) {
        in = 1;
        foundDevices++;
        }
        
    if (status == 0)
        return foundDevices;
    
    name = snd_rawmidi_info_get_name(info);
    sub_name = snd_rawmidi_info_get_subdevice_name(info);
    if (sub_name[0] == '\0') {
        m_availableDevices.insert(name,QString("hw:%1,%2").arg(card).arg(device));
        devices.append(name);
    } else {
        sub = 0;
        for (;;) {
            m_availableDevices.insert(sub_name,QString("hw:%1,%2,%3").arg(card).arg(device).arg(sub));
            devices.append(sub_name);
            if (++sub >= subs)
                break;
    
            in = is_input(ctl, card, device, sub);
            out = is_output(ctl, card, device, sub);
            snd_rawmidi_info_set_subdevice(info, sub);
            if (out) {
                snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
                if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0) {
                    qWarning() << QString("Cannot get RawMIDI information %d:%d:%d: %s")
                        .arg(card).arg(device).arg(sub).arg(snd_strerror(status));
                    break;
                }
            } else {
                snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
                if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0) {
                qWarning() << QString("Cannot get RawMIDI information %d:%d:%d: %s")
                        .arg(card).arg(device).arg(sub).arg(snd_strerror(status));
                break;
                }
            }
            sub_name = snd_rawmidi_info_get_subdevice_name(info);
        }
    }
    return foundDevices;
}

//////////////////////////////
//
// is_input -- returns true if specified card/device/sub can output MIDI data.
//

int MidiObjectALSA::is_input(snd_ctl_t *ctl, int card, int device, int sub) {
   snd_rawmidi_info_t *info;
   int status;

   snd_rawmidi_info_alloca(&info);
   snd_rawmidi_info_set_device(info, device);
   snd_rawmidi_info_set_subdevice(info, sub);
   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
   
   if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0 && status != -ENXIO) {
      return status;
   } else if (status == 0) {
      return 1;
   }

   return 0;
}



//////////////////////////////
//
// is_output -- returns true if specified card/device/sub can output MIDI data.
//

int MidiObjectALSA::is_output(snd_ctl_t *ctl, int card, int device, int sub) {
   snd_rawmidi_info_t *info;
   int status;

   snd_rawmidi_info_alloca(&info);
   snd_rawmidi_info_set_device(info, device);
   snd_rawmidi_info_set_subdevice(info, sub);
   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
   
   if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0 && status != -ENXIO) {
      return status;
   } else if (status == 0) {
      return 1;
   }

   return 0;
}
