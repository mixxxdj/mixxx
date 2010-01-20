/***************************************************************************
                          soundsourceproxy.h  -  description
                             -------------------
    begin                : Wed Oct 13 2004
    copyright            : (C) 2004 by Tue Haste Andersen
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

#ifndef SOUNDSOURCEPROXY_H
#define SOUNDSOURCEPROXY_H

#include "soundsource.h"
//Added by qt3to4:
#include <Q3ValueList>

class TrackInfoObject;

/**
  *@author Tue Haste Andersen
  */

/*
  Base class for sound sources.
*/
class SoundSourceProxy : public SoundSource
{
public:
    SoundSourceProxy(QString qFilename);
    SoundSourceProxy(TrackInfoObject *pTrack);
    ~SoundSourceProxy();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    long unsigned length();
    static int ParseHeader(TrackInfoObject *p);
    unsigned int getSrate();
    Q3ValueList<long> *getCuePoints();
    /** Returns filename */
    QString getFilename();

private:
    void initialize(QString qFilename);

    SoundSource *m_pSoundSource;
};

#endif
