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
#include "enginemaster.h"
#include "controlobject.h"

short int Player::m_iBufferSize = 0;
short int Player::m_iChannels = 0;
EngineMaster *Player::m_pMaster = 0;

/* -------- ------------------------------------------------------
   Purpose: Initializes the audio hardware.
   Input:   Size of the output buffer in samples
   Output:  Pointer to internal synthesis data structure.
   -------- ------------------------------------------------------ */
Player::Player(ConfigObject<ConfigValue> *pConfig)
{
    m_pConfig = pConfig;
    m_pBuffer = new CSAMPLE[MAX_BUFFER_LEN];
    m_pControlObjectSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));
}

/* -------- ------------------------------------------------------
   Purpose: Terminate and deallocate the synthesis system
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
Player::~Player()
{
    delete [] m_pBuffer;
}

short int Player::getBufferSize()
{
    return m_iBufferSize/m_iChannels;
}

void Player::setMaster(EngineMaster *pMaster)
{
    m_pMaster = pMaster;
}

bool Player::open()
{
    if (m_pConfig->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt())
        qDebug("pitch true");
        
    // Set sound scale method
    if (m_pMaster)
        m_pMaster->setPitchIndpTimeStretch(m_pConfig->getValueString(ConfigKey("[Soundcard]","PitchIndpTimeStretch")).toInt());

    return true;
}

/* -------- ------------------------------------------------------
   Purpose: Internal callback function used for preparing samples
            for playback. This is where the synthesis is done.
   Input:   Number of samples for each channel (4 channels in all)
   Output:  pointer to resulting buffer of samples
   -------- ------------------------------------------------------ */
CSAMPLE *Player::prepareBuffer(int iBufferSize)
{
    // First, sync control parameters with changes from GUI thread
    ControlObject::sync();

    // Process a block of samples for output. iBufferSize is the
    // number of samples for one channel, but the EngineObject
    // architecture expects number of samples for two channels
    // as input so...
    m_pMaster->process(0, m_pBuffer, iBufferSize*2);
    return m_pBuffer;
}

