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

#ifdef RECORD_OUTPUT
  // HACK TO RECORD OUTPUT. WRITTEN TO FILE AT PROGRAM EXIT
  #include <audiofile.h>
  #include <q3ptrlist.h>
  typedef struct {
      short int *pBuffer;
      int size;
  } recordObject;
  Q3PtrList<recordObject> m_qRecordList;
#endif

//Static variable memory allocation
short int Player::m_iBufferSize = 0;
short int Player::m_iChannels[MAX_AUDIODEVICES];
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
    m_pControlObjectLatency = new ControlObject(ConfigKey("[Master]","latency"));;
    
    for (int i = 0; i < MAX_AUDIODEVICES; i++)
    {
    	m_iChannels[i] = 0;
	}
}

/* -------- ------------------------------------------------------
   Purpose: Terminate and deallocate the synthesis system
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
Player::~Player()
{
    delete [] m_pBuffer;
    
#ifdef RECORD_OUTPUT
    //
    // HACK: Write recorded sound
    //

    qDebug("Writing output to file... please wait!");

    // Setup file format
    AFfilesetup outputSetup = afNewFileSetup();
    afInitFileFormat(outputSetup, AF_FILE_WAVE);
    afInitRate(outputSetup, AF_DEFAULT_TRACK, m_pControlObjectSampleRate->get());
    afInitChannels(outputSetup, AF_DEFAULT_TRACK, 2);
    afInitSampleFormat (outputSetup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);
    //afInitByteOrder (outputSetup, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);

    // Open file handle
    AFfilehandle fh = afOpenFile("output.wav","w",outputSetup);
    if (!fh)
        qFatal("Could not write wave output file");
    recordObject *r;
    for (r = m_qRecordList.first(); r; r=m_qRecordList.next())
    {
        afWriteFrames(fh,AF_DEFAULT_TRACK,r->pBuffer,r->size/2);
    }
    afCloseFile(fh);
#endif
}

/*
short int Player::getBufferSize()
{
//     if (m_iChannels>0)
//         return m_iBufferSize/m_iChannels;
//     else
        return m_iBufferSize;
}
*/

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
    
#ifdef RECORD_OUTPUT
    // HACK: RECORD SOUND
    recordObject *r = new recordObject;
    m_qRecordList.append(r);
    r->size = iBufferSize*2;
    r->pBuffer = new short int[r->size];
    for (int j=0; j<r->size; ++j)
        r->pBuffer[j] = (short int)(m_pBuffer[j]);
#endif

    return m_pBuffer;
}

