/***************************************************************************
                          soundbufferevent.h  -  description
                             -------------------
    begin                : Mon Mar 3 2003
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

#ifndef SOUNDBUFFEREVENT_H
#define SOUNDBUFFEREVENT_H

#include <qevent.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class SoundBufferEvent : public QCustomEvent
{
public: 
    SoundBufferEvent(int pos, int len);
    ~SoundBufferEvent();
    int pos() const;
    int len() const;
private:
    int position;
    int length;
};

#endif
