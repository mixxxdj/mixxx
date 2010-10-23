/****************************************************************************
                   encoder.h  - encoder API for mixxx
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

#ifndef ENCODER_H
#define ENCODER_H

#include <stdlib.h> // needed for random num gen
#include <time.h> // needed for random num gen
#include <string.h> // needed for memcpy
#include <QDebug>

#include "engine/engineabstractrecord.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "playerinfo.h"
#include "trackinfoobject.h"


class Encoder : public QObject {
    Q_OBJECT

  public:
    Encoder();
    virtual ~Encoder();
    /*
     * General Encoder methods
     */
    virtual int initEncoder(int bitrate) = 0;
    virtual void encodeBuffer(const CSAMPLE *samples, const int size) = 0;
    virtual void updateMetaData(char* artist, char* title, char* album) = 0;
    virtual void flush() = 0;
    /**converts an OGG quality measure from 1..10 to a bitrate **/
    static int convertToBitrate(int quality);
};

#endif // ENCODER_H
