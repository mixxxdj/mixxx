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

EngineMaster *Player::m_pMaster = 0;

/* -------- ------------------------------------------------------
   Purpose: Initializes the audio hardware.
   Input:   Size of the output buffer in samples
   Output:  Pointer to internal synthesis data structure.
   -------- ------------------------------------------------------ */
Player::Player(ConfigObject<ConfigValue> *pConfig, ControlObject *pControl)
{
    m_pConfig = pConfig;
    m_pControl = pControl;

//    qDebug("Player: init...");
}

/* -------- ------------------------------------------------------
   Purpose: Terminate and deallocate the synthesis system
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
Player::~Player()
{
}

void Player::setMaster(EngineMaster *pMaster)
{
    m_pMaster = pMaster;
}

bool Player::open()
{
    // Set sound quality in engine
    if (m_pMaster)
        m_pMaster->setQuality(m_pConfig->getValueString(ConfigKey("[Soundcard]","SoundQuality")).toInt());

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
    m_pControl->syncControlEngineObjects();

    // Process a block of samples for output. iBufferSize is the
    // number of samples for one channel, but the EngineObject
    // architecture expects number of samples for two channels
    // as input so...
    return m_pMaster->process(0, iBufferSize*2);
}
