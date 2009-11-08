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
#include "mixxxevent.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class ReaderEvent : public QEvent
{
public:
    ReaderEvent(int bpos, int blen, long int fspos, int bspos, int flen, int srate);
    ~ReaderEvent();
    int bufferPos() const;
    int bufferLen() const;
    int fileLen() const;
    long int fileStartPos() const;
    int bufferStartPos() const;
    int srate() const;
    
private:
    int bufferPosition;
    int bufferLength;
    long int fileStartPosition;
    int bufferStartPosition;
    int fileLength;
    int sampleRate;
};

#endif
