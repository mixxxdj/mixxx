/***************************************************************************
                          controlvumeter.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTROLVUMETER_H
#define CONTROLVUMETER_H

#include "controlobject.h"
#include "configobject.h"

class WBulb;
class DlgVUmeter;

/**
  *@author Tue and Ken Haste Andersen
  */

#define NLEDS 6

class ControlVUmeter : public ControlObject
{
    Q_OBJECT
public:
    ControlVUmeter(ConfigKey key, DlgVUmeter *);
    ~ControlVUmeter();
    void setValue(FLOAT_TYPE);
    void setAccelUp(const QKeySequence key);
    void setAccelDown(const QKeySequence key);


public slots:
    void slotSetPosition(int);
    void slotSetPositionMidi(MidiCategory c, int v);

signals:
    void valueChanged(FLOAT_TYPE);

protected:
    void forceGUIUpdate();

private:
    static const FLOAT_TYPE m_fTresholds[NLEDS];
    FLOAT_TYPE m_fValue;
    WBulb *m_aLeds[NLEDS];

};


#endif
