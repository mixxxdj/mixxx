/***************************************************************************
                          dlgpreferences.cpp  -  description
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

#include "dlgpreferences.h"
#ifdef __ALSA__
  #include "playeralsa.h"
#endif
#ifdef __PORTAUDIO__
  #include "playerportaudio.h"
#endif
#include "dlgprefsound.h"
#include "dlgprefmidi.h"
#include "dlgprefplaylist.h"
#include "dlgprefcontrols.h"
#include "mixxx.h"
#include "tracklist.h"

DlgPreferences::DlgPreferences(MixxxApp *mixxx, MixxxView *view,
                               MidiObject *midi, Player *player,
                               TrackList *tracklist, ConfigObject<ConfigValue> *config,
                               ConfigObject<ConfigValueMidi> *midiconfig,
                               ControlObject *pControl) : QTabDialog(mixxx, "")
{    
    setCaption("Preferences");
    
    // Construct widgets for use in tabs
    wsound = new DlgPrefSound(this, player, config);
    wmidi  = new DlgPrefMidi(this, midi, config, midiconfig);
    wplaylist = new DlgPrefPlaylist(this, config);
    wcontrols = new DlgPrefControls(this, pControl, view, config);
    
    // Add tabs
    addTab(wsound,    "Sound output");
    addTab(wmidi,     "MIDI");
    addTab(wcontrols, "GUI");
    addTab(wplaylist, "Playlists");    

    // Add closebutton
    setOkButton("Close");

    // Set size
    resize(QSize(380,500));

    // Install event handler to generate closeDlg signal
    installEventFilter(this);
    
    // Connections
    connect(this,        SIGNAL(aboutToShow()),          wsound,    SLOT(slotUpdate()));
    connect(this,        SIGNAL(aboutToShow()),          wmidi,     SLOT(slotUpdate()));
    connect(this,        SIGNAL(aboutToShow()),          wplaylist, SLOT(slotUpdate()));
    connect(this,        SIGNAL(aboutToShow()),          wcontrols, SLOT(slotUpdate()));
    connect(this,        SIGNAL(closeDlg()),             wsound,    SLOT(slotApply()));
    connect(this,        SIGNAL(closeDlg()),             wmidi,     SLOT(slotApply()));
    connect(this,        SIGNAL(closeDlg()),             wplaylist, SLOT(slotApply()));
    connect(this,        SIGNAL(closeDlg()),             wcontrols, SLOT(slotApply()));
    connect(wplaylist,   SIGNAL(apply(QString)),         tracklist, SLOT(slotUpdateTracklist(QString)));
}

DlgPreferences::~DlgPreferences()
{
}

bool DlgPreferences::eventFilter(QObject *o, QEvent *e)
{
    // Send a close signal if dialog is closing
    if (e->type() == QEvent::Hide)
        emit(closeDlg());

    // Standard event processing
    return QWidget::eventFilter(o,e);
}

