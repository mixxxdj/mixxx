/***************************************************************************
                          player.h  -  description
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

#ifndef PLAYER_H
#define PLAYER_H

#include <qobject.h>
#include "defs.h"
#include <stdlib.h>
#include <iostream>
#include "engineobject.h"
#include "controlobject.h"
#include <vector>
#include <qvaluelist.h>
#include <qptrlist.h>


class Player : public EngineObject {
public:
    Player(int, std::vector<EngineObject *> *);
	~Player();      // Deallocate
    bool reopen(QString name, int srate, int bits, int bufferSize);
	virtual void start(EngineObject *); // Start audio stream
	virtual void stop() = 0;           // Stops audio stream
	virtual void wait() = 0;           // Wait for audio stream to finish

    typedef struct
    {
        QString         name;
        QValueList<int> sampleRates;
        QValueList<int> bits;
        QValueList<int> bufferSizes;
    } Info;

    QPtrList<Info> *getInfo();

    SAMPLE *out_buffer;
	int prepareBuffer(); // Calculates one buffer of sound
	int buffer_size;

protected:
    virtual bool open(QString name, int srate, int bits, int bufferSize) = 0;
    virtual void close() = 0;
	void allocate();
	void deallocate();

	std::vector<EngineObject *> *engines;
	CSAMPLE *process_buffer,*tmp1, *tmp2;
	int index;    // Current playback frame in input buffer
	EngineObject* reader;
    QPtrList<Info>  devices;
};

#endif
