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

#include "encoder/encoder.h"

Encoder::Encoder() {
}

Encoder::~Encoder() {
}

int Encoder::convertToBitrate(int quality) {
    switch(quality)
    {
        case 1: return 48;
        case 2: return 64;
        case 3: return 80;
        case 4: return 96;
        case 5: return 112;
        case 6: return 128;
        case 7: return 160;
        case 8: return 192;
        case 9: return 224;
        case 10: return 256;
        case 11: return 320;
        default: return 128;
    }
}
