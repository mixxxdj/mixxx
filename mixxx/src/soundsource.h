/***************************************************************************
                          soundsource.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
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

#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H

#include "defs.h"
#include <qstring.h>

class TrackInfoObject;

/**
  *@author Tue and Ken Haste Andersen
  */

/*
  Base class for sound sources.
*/
class SoundSource
{
public:
    SoundSource(QString qFilename);
    virtual ~SoundSource();
    virtual long seek(long) = 0;
    virtual unsigned read(unsigned long size, const SAMPLE*) = 0;
    virtual long unsigned length() = 0;
    static int ParseHeader(TrackInfoObject *);
    virtual int getSrate();
    /** Returns filename */
    virtual QString getFilename();

protected:
    /** Sample rate of the file */
    int SRATE;
    /** File name */
    QString m_qFilename;
};

#endif
