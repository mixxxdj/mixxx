/***************************************************************************
                          controlevent.h  -  description
                             -------------------
    begin                : Mon Sep 27 2004
    copyright            : (C) 2004 by Tue Haste Andersen
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

#ifndef CONTROLEVENT_H
#define CONTROLEVENT_H

#include <qevent.h>
#include "mixxxevent.h"


/**
  *@author Tue Haste Andersen
  *
  * Event used in communication from ControlObject to ControlObjectThreadMain
  *
  */

class ControlEvent : public QEvent {
public: 
    ControlEvent(double dValue);
    ~ControlEvent();
    double value() const;
private:
    double m_dValue;
};

#endif
