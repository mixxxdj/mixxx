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
class PowerMate;
class Mouse;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefMidi : public DlgPrefMidiDlg  {
    Q_OBJECT
public:
    DlgPrefMidi(QWidget *parent, MidiObject *pMidi, ConfigObject<ConfigValue> *pConfig,
                ConfigObject<ConfigValueMidi> *pMidiConfig,
                PowerMate *pPowerMate1, PowerMate *pPowerMate2);
    ~DlgPrefMidi();
public slots:
    void slotUpdate();
    void slotApply();
    void slotMouseCalibrate1();
    void slotMouseCalibrate2();
    void slotMouseHelp();
signals:
    void apply();
private:
    MidiObject *m_pMidi;
    ConfigObject<ConfigValue> *m_pConfig;
    ConfigObject<ConfigValueMidi> *m_pMidiConfig;
    PowerMate *m_pPowerMate1, *m_pPowerMate2;
    Mouse *m_pMouse1, *m_pMouse2;
};

#endif
