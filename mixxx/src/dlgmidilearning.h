/***************************************************************************
                          dlgmidilearning.cpp  -  description
                             -------------------
    begin                : Mon March 2 2009
    copyright            : (C) 2009 by Albert Santoni
    email                : alberts at mixxx dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGMIDILEARNING_H
#define DLGMIDILEARNING_H

#include <QtCore>
#include <QtGui>
#include "ui_dlgmidilearning.h"
#include "configobject.h"
#include "mixxxcontrol.h"
#include "midi/midimessage.h"

class MidiMapping;

/**
  *@author Albert Santoni
  */

class DlgMidiLearning : public QDialog, public Ui::DlgMidiLearning {
    Q_OBJECT
public:
    DlgMidiLearning(QWidget *parent, MidiMapping* mapping);
    ~DlgMidiLearning();
public slots:
    void begin();   /** Begin the MIDI learning phase */
    void next();    /** Ask to map the next control */
    void prev();    /** Ask to map the previous control */
    void controlMapped(MidiMessage); /** Gets called when a control has just been mapped successfully */
private:
    MidiMapping* m_pMidiMapping;
    QList<MixxxControl> m_controlsToBind;
    QList<QString> m_controlDescriptions;
    int iCurrentControl; /** Used to iterate through the controls list */
    QShortcut* m_pSkipShortcut;

};

#endif
