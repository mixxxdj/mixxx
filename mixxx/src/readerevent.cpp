/***************************************************************************
                          readerevent.cpp  -  description
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

#include "readerevent.h"
//Added by qt3to4:
#include <QCustomEvent>

ReaderEvent::ReaderEvent(int bpos, int blen, long int fspos, int bspos, int flen, int srate) : QCustomEvent(10002), bufferPosition(bpos), bufferLength(blen), fileStartPosition(fspos), bufferStartPosition(bspos), fileLength(flen), sampleRate(srate)
{
}

ReaderEvent::~ReaderEvent()
{
}

int ReaderEvent::bufferPos() const
{
    return bufferPosition;
}

int ReaderEvent::bufferLen() const
{
    return bufferLength;
}

long int ReaderEvent::fileStartPos() const
{
    return fileStartPosition;
}

int ReaderEvent::bufferStartPos() const
{
    return bufferStartPosition;
}

int ReaderEvent::fileLen() const
{
    return fileLength;
}

int ReaderEvent::srate() const
{
    return sampleRate;
}
