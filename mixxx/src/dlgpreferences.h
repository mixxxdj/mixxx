/***************************************************************************
                          dlgpreferences.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
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

#ifndef DLGPREFERENCES_H
#define DLGPREFERENCES_H

#include "dlgpreferencesdlg.h"
#include <qwidget.h>
#include "midiobject.h"
#include "player.h"
#include "configobject.h"
#include <qstringlist.h>

class MixxxApp;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPreferences : public DlgPreferencesDlg
{
    Q_OBJECT
public: 
    DlgPreferences(MixxxApp *_mixxx, const char *name,
                   MidiObject *_midi, Player *_player, Player *_playerSlave,
                   ConfigObject<ConfigValue> *_config,
                   ConfigObject<ConfigValueMidi> *_midiconfig);
    ~DlgPreferences();

public slots:
    /** Update QComboBox values when devices are changed */
    void slotMasterDevice();
    void slotHeadDevice();
    void slotMasterDeviceOptions();
    void slotHeadDeviceOptions();
    void slotLatencyMaster();
    void slotLatencyHead();
    /** Apply preferences */
    void slotApply();
    /** Set preferences from dialog */
    void slotSetPreferences();
protected:
    void closeEvent(QCloseEvent *e);
private:
    MixxxApp *mixxx;
    MidiObject *midi;
    Player *player, *playerSlave;
    ConfigObject<ConfigValue> *config;
    ConfigObject<ConfigValueMidi> *midiconfig;
    /** Transform a slider value to latency value in msec */
    int getSliderLatencyMsec(int);
    /** Transform latency value in msec to slider value */
    int getSliderLatencyVal(int);
    void initMidiConfigList();

};

#endif
