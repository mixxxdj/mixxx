/***************************************************************************
                          dlgprefmidi.cpp  -  description
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#include "dlgprefmidi.h"
#include "midiobject.h"
#include <qcombobox.h>

DlgPrefMidi::DlgPrefMidi(QWidget *parent, MidiObject *_midi, ConfigObject<ConfigValue> *_config,
                         ConfigObject<ConfigValueMidi> *_midiconfig) : DlgPrefMidiDlg(parent,"")
{
    midi = _midi;
    config = _config;
    midiconfig = _midiconfig;
}

DlgPrefMidi::~DlgPrefMidi()
{
}

void DlgPrefMidi::slotUpdate()
{
    // Midi configurations
    ComboBoxMidiconf->clear();
    QStringList *midiConfigList = midi->getConfigList(config->getValueString(ConfigKey("[Midi]","Configdir")));
    int j=0;
    if (midiConfigList->count()>0)
    {
        for (QStringList::Iterator it = midiConfigList->begin(); it != midiConfigList->end(); ++it )
        {
            // Insert the file name into the list, with ending (.midi.cfg) stripped
            ComboBoxMidiconf->insertItem((*it).left((*it).length()-9));

            if ((*it) == config->getValueString(ConfigKey("[Midi]","Configfile")))
                ComboBoxMidiconf->setCurrentItem(j);
            j++;
        }
    }

    // Midi devices
    ComboBoxMididevice->clear();
    QStringList *midiDeviceList = midi->getDeviceList();
    j=0;
    for (QStringList::Iterator it = midiDeviceList->begin(); it != midiDeviceList->end(); ++it )
    {
        ComboBoxMididevice->insertItem(*it);
        if ((*it) == (*midi->getOpenDevice()))
            ComboBoxMididevice->setCurrentItem(j);
        j++;
    }
}

void DlgPrefMidi::slotApply()
{
    config->set(ConfigKey("[Midi]","Configfile"), ConfigValue(ComboBoxMidiconf->currentText().append(".midi.cfg")));
    config->set(ConfigKey("[Midi]","Device"), ConfigValue(ComboBoxMididevice->currentText()));

    // Close MIDI
    midi->devClose();

    // Change MIDI configuration
    //midiconfig->clear(); // (is currently not implemented correctly)
    midiconfig->reopen(config->getValueString(ConfigKey("[Midi]","Configdir")).append(config->getValueString(ConfigKey("[Midi]","Configfile"))));

    // Open MIDI device
    midi->devOpen(config->getValueString(ConfigKey("[Midi]","Device")));

    // Save preferences
    config->Save();
}

