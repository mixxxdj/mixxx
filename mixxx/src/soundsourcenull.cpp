/***************************************************************************
                          soundsourcenull.cpp  -  description
                             -------------------
    begin                : Tue Feb 26 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#include "soundsourcenull.h"

SoundSourceNull::SoundSourceNull()
{
}

SoundSourceNull::~SoundSourceNull()
{
}

long SoundSourceNull::seek(long filepos)
{
	return filepos;
}

unsigned SoundSourceNull::read(unsigned long size, const SAMPLE* _destination)
{
	//SAMPLE *destination = (SAMPLE*)_destination;

//	for (int i=0; i<size; i++)
//		destination[i] = 0;
	return size;
}

long unsigned SoundSourceNull::length()
{
	return 31410432;
}

