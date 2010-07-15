/****************************************************************************
                   encoder.cpp  - encoder API for mixxx
                             -------------------
    copyright            : (C) 2009 by Phillip Whelan
    copyright            : (C) 2010 by Tobias Rafreider
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h> // needed for random num gen
#include <time.h> // needed for random num gen
#include <string.h> // needed for memcpy
#include <QDebug>

#include "engine/engineabstractrecord.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "playerinfo.h"
#include "trackinfoobject.h"

#include "encoder.h"

// Constructor
Encoder::Encoder()
{
}

// Destructor
Encoder::~Encoder()
{
}
int Encoder::convertToBitrate(int quality){
	switch(quality)
        {
            case 1: return 16;
            case 2: return 24;
            case 3: return 32;
            case 4: return 64;
            case 5: return 128;
            case 6: return 160;
            case 7: return 192;
            case 8: return 224;
            case 9: return 256;
            case 10: return 320;
			default: return 128;
        }
}
