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
#include "enginebuffer.h"
#include "controlobject.h"
#include <qmultilineedit.h>

class Player : public QObject {
public:
	Player(int);
	~Player();      // Deallocate
	virtual void start(EngineBuffer*); // Start audio stream
	virtual void stop() = 0;           // Stops audio stream
	virtual void wait() = 0;           // Wait for audio stream to finish

	SAMPLE *out_buffer;
	int prepareBuffer(); // Calculates one buffer of sound
	int BUFFER_SIZE;

protected:
	void allocate();
	void deallocate();

	CSAMPLE *process_buffer,*tmp1, *tmp2;
	int index;    // Current playback frame in input buffer
	EngineBuffer* reader;
	unsigned long int play_pos;
	QMultiLineEdit *messages;
};

#endif
