/***************************************************************************
                          windowkaiser.h  -  description
                             -------------------
    begin                : Sat Feb 8 2003
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

#ifndef WINDOWKAISER_H
#define WINDOWKAISER_H

#include "defs.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class WindowKaiser 
{
public: 
    WindowKaiser(int _size, CSAMPLE beta);
    ~WindowKaiser();
    int getSize();
    CSAMPLE *getWindowPtr();
    CSAMPLE getAFactor();
private:
    int size;
    CSAMPLE afactor;
    CSAMPLE *window;
};

#endif
