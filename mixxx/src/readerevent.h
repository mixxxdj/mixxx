/***************************************************************************
                          readerevent.h  -  description
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

#ifndef READEREVENT_H
#define READEREVENT_H

#include <qevent.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class ReaderEvent : public QCustomEvent
{
public:
    ReaderEvent(int bpos, int blen, int flen, int srate);
    ~ReaderEvent();
    int bufferPos() const;
    int bufferLen() const;
    int fileLen() const;
    int srate() const;
private:
    int bufferPosition;
    int bufferLength;
    int fileLength;
    int sampleRate;
};

#endif
