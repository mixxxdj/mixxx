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

//#include "dlgpreferencesdlg.h"
#include <qtabdialog.h>
#include <qevent.h>
#include "configobject.h"

class MixxxApp;
class MixxxView;
class MidiObject;
class Player;
class TrackList;
class DlgPrefSound;
class DlgPrefMidi;
class DlgPrefPlaylist;
class DlgPrefControls;
class ControlObject;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPreferences : public QTabDialog
{
    Q_OBJECT
public: 
    DlgPreferences(MixxxApp *mixxx, MixxxView *view,
                   MidiObject *midi, Player *player,
                   TrackList *tracklist, ConfigObject<ConfigValue> *config,
                   ConfigObject<ConfigValueMidi> *midiconfig,
                   ControlObject *pControl);
    ~DlgPreferences();
signals:
    void closeDlg();
protected:
    bool eventFilter(QObject *, QEvent *);
private:
    DlgPrefSound *wsound;
    DlgPrefMidi *wmidi;
    DlgPrefPlaylist *wplaylist;
    DlgPrefControls *wcontrols;
    ConfigObject<ConfigValue> *config;
};

#endif
