/***************************************************************************
                          joystick.cpp  -  description
                             -------------------
    begin                : Thu Jul 10 2003
    copyright            : (C) 2003 by Svein Magne Bang
    email                : sveinmb@stud.ntnu.no
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "controlobject.h"
#include "controleventmidi.h"
#include "qapplication.h"
#include "midiobject.h"
 
#include "joystick.h"


Joystick::Joystick(ControlObject *pControl){
    m_pControl = pControl;
}

Joystick::~Joystick(){
}
