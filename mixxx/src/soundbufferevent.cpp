/***************************************************************************
                          soundbufferevent.cpp  -  description
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

#include "soundbufferevent.h"

SoundBufferEvent::SoundBufferEvent(int pos, int len) : QCustomEvent(10002), position(pos), length(len)
{
}

SoundBufferEvent::~SoundBufferEvent()
{
}

int SoundBufferEvent::pos() const
{
    return position;
}

int SoundBufferEvent::len() const
{
    return length;
}
