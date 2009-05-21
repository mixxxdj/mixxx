/***************************************************************************
                          dlgprefnomidi.h  -  "No MIDI devices available"
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

#ifndef DLGPREFNOMIDI_H
#define DLGPREFNOMIDI_H

#include "ui_dlgprefnomididlg.h"
#include "configobject.h"

//class QWidget;

class DlgPrefNoMidi : public QWidget, public Ui::DlgPrefNoMidiDlg  {
    Q_OBJECT
public:
    DlgPrefNoMidi(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefNoMidi();
};

#endif
