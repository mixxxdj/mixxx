/***************************************************************************
                          wslider.h  -  description
                             -------------------
    begin                : Mon Jun 23 2003
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

#ifndef WSLIDER_H
#define WSLIDER_H

#include "wknob.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class WSlider : public WKnob  {
public: 
    WSlider(QWidget *parent=0);
    ~WSlider();
};

#endif
