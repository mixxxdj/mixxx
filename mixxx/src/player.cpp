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

// Static member variable definition
SAMPLE *Player::out_buffer = 0;
SAMPLE *Player::out_buffer_offset = 0;

/* -------- ------------------------------------------------------
   Purpose: Initializes the audio hardware.
   Input:   Size of the output buffer in samples
   Output:  Pointer to internal synthesis data structure.
   -------- ------------------------------------------------------ */
Player::Player(int, std::vector<EngineObject*> *_engines, QString)
{
    engines = _engines;

//    qDebug("Player: init...");
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

bool Player::reopen(QString name, int srate, int bits, int bufferSize, int chMaster, int chHead)
{
    close();
    return open(name,srate,bits,bufferSize,chMaster,chHead);
}

void Player::allocate()
{
    // Allocate buffer
    out_buffer = new SAMPLE[MAX_BUFFER_LEN*10];
    out_buffer_offset = out_buffer;
    
    bufferIdx = 0;
}

void Player::deallocate()
{
    delete [] out_buffer;
}

/* -------- ------------------------------------------------------
   Purpose: Start the audio stream
   Input:   Internal synth datastructure
   Output:
   -------- ------------------------------------------------------ */
void Player::start(EngineObject *_reader)
{
    reader = _reader;
}

/* -------- ------------------------------------------------------
   Purpose: Internal callback function used for preparing samples
            for playback. This is where the synthesis is done.
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
int Player::prepareBuffer()
{
  // ----------------------------------------------------
  // Do the processing.
  // ----------------------------------------------------

  CSAMPLE *p1, *p2;

  // Resample; the linear interpolation is done in readfile:
  p1 = reader->process(0, BUFFERSIZE);

  {for (unsigned int i=0; i<engines->size(); i++)
  {
      p2 = (*engines)[i]->process(p1, BUFFERSIZE);
      p1=p2;
  }}

  // Convert the signal back to SAMPLE and write to the sound cards buffer:
  if (bufferIdx>20)
      bufferIdx = 0;
  else
      bufferIdx++;
  out_buffer_offset = out_buffer + (BUFFERSIZE*2*bufferIdx);
  for (int i=0; i<BUFFERSIZE*2; i++)
      out_buffer_offset[i] = (SAMPLE)p1[i];

  return 0;
}

QPtrList<Player::Info> *Player::getInfo()
{
    return &devices;
}

