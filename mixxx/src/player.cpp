/***************************************************************************
                          player.cpp  -  description
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

#include "player.h"

/* -------- ------------------------------------------------------
   Purpose: Initializes the audio hardware.
   Input:   Size of the output buffer in samples
   Output:  Pointer to internal synthesis data structure.
   -------- ------------------------------------------------------ */
Player::Player(int size, std::vector<EngineObject*> *_engines)
{
    engines = _engines;
    qDebug("Player: init...");
}

/* -------- ------------------------------------------------------
   Purpose: Terminate and deallocate the synthesis system
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
Player::~Player()
{
	deallocate();
}

void Player::allocate()
{
	// Allocate buffers
	out_buffer = new SAMPLE[BUFFER_SIZE];
	process_buffer = new CSAMPLE[BUFFER_SIZE];
	tmp1 = new CSAMPLE[BUFFER_SIZE];
	tmp2 = new CSAMPLE[BUFFER_SIZE];
}

void Player::deallocate()
{
	delete [] tmp2;
	delete [] tmp1;
	delete [] process_buffer;
	delete [] out_buffer;
}

/* -------- ------------------------------------------------------
   Purpose: Start the audio stream
   Input:   Internal synth datastructure
   Output:
   -------- ------------------------------------------------------ */
void Player::start(EngineObject *_reader) {
	reader = _reader;
}

/* -------- ------------------------------------------------------
   Purpose: Internal callback function used for preparing samples
            for playback. This is where the synthesis is done.
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
int Player::prepareBuffer() {
  // ----------------------------------------------------
  // Do the processing.
  // ----------------------------------------------------

  CSAMPLE *p1, *p2;

  //qDebug("player::prepareBuffer()");
  // Resample; the linear interpolation is done in readfile:
  p1 = reader->process(0, buffer_size);

  for (unsigned int i=0; i<engines->size(); i++)
  {
      p2 = (*engines)[i]->process(p1, buffer_size);
      p1=p2;
  }

  // Convert the signal back to SAMPLE and write to the sound cards buffer:
  for (int i=0; i<buffer_size; i++)
    out_buffer[i] = (SAMPLE)p1[i];

  return 0; // Hack. Should only return 0 when not at end of file
}
