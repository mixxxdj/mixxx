/***************************************************************************
                          readerbufferevent.h  -  description
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

#ifndef READERBUFFEREVENT_H
#define READERBUFFEREVENT_H

#include <qevent.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class ReaderBufferEvent : public QCustomEvent
{
public: 
    ReaderBufferEvent(int pos, int len);
    ~ReaderBufferEvent();
    int pos() const;
    int len() const;
private:
    int position;
    int length;
};

#endif
