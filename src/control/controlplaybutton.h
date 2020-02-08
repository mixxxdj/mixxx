/***************************************************************************
                          controlplayhbutton.h  -  description
                             -------------------
    begin                : Sat Feb 29 2020
    copyright            : (C) 2020 by Philip Gottschling
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

#ifndef CONTROLPLAYBUTTON_H
#define CONTROLPLAYBUTTON_H

#include "control/controlpushbutton.h"

/**
  *@author Philip Gottschling
  */

class ControlPlayButton : public ControlPushButton {
    Q_OBJECT
  public:
    ControlPlayButton(QString group);
    virtual ~ControlPlayButton();
};

#endif // CONTROLPLAYBUTTON_H
