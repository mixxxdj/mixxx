/***************************************************************************
                          dlgprefmidi.h  -  description
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

#ifndef DLGPREFMIDI_H
#define DLGPREFMIDI_H

#include <qwidget.h>
#include "dlgprefmididlg.h"
#include "configobject.h"

class MidiObject;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefMidi : public DlgPrefMidiDlg  {
    Q_OBJECT
public: 
    DlgPrefMidi(QWidget *parent, MidiObject *_midi, ConfigObject<ConfigValue> *_config,
                ConfigObject<ConfigValueMidi> *_midiconfig);
    ~DlgPrefMidi();
public slots:
    void slotUpdate();
    void slotApply();
signals:
    void apply();
private:
    MidiObject *midi;
    ConfigObject<ConfigValue> *config;
    ConfigObject<ConfigValueMidi> *midiconfig;
};

#endif
