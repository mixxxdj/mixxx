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
int Player::HeadPerMasterBuffer = 1;
int Player::MasterBufferSize = 0;

/* -------- ------------------------------------------------------
   Purpose: Initializes the audio hardware.
   Input:   Size of the output buffer in samples
   Output:  Pointer to internal synthesis data structure.
   -------- ------------------------------------------------------ */
Player::Player(ConfigObject<ConfigValue> *_config)
{
    config = _config;
    allocate();

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

bool Player::open(bool useDefault)
{
    if (useDefault)
    {
        //
        // Open master using default device, and overwrite config data
        //
        if (open(getDefaultDevice(),44100,16,BUFFER_SIZE,1,0))
            config->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue(44100));
        else if (open(getDefaultDevice(),48000,16,BUFFER_SIZE,1,0))
            config->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue(48000));
        else if (open(getDefaultDevice(),22050,16,BUFFER_SIZE,1,0))
            config->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue(22050));
        else
        {
            // Reset config data for master channel
            config->set(ConfigKey("[Soundcard]","ChannelMaster"),ConfigValue(""));
            config->set(ConfigKey("[Soundcard]","DeviceMaster"),ConfigValue(""));

            return false;
        }

        config->set(ConfigKey("[Soundcard]","ChannelMaster"),ConfigValue(1));
        config->set(ConfigKey("[Soundcard]","DeviceMaster"),ConfigValue(getDefaultDevice()));
        config->set(ConfigKey("[Soundcard]","Bits"),ConfigValue(16));
        config->set(ConfigKey("[Soundcard]","LatencyMaster"),ConfigValue((int)((MasterBufferSize*2)/((float)config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt()/1000.))));

        // Reset config data for headphone channel
        config->set(ConfigKey("[Soundcard]","ChannelHeadphone"),ConfigValue(""));
        config->set(ConfigKey("[Soundcard]","DeviceHeadphone"),ConfigValue(""));
    
    }
    else
    {
        //
        // Open devices using information from config object. If anything fails return false.
        // Latency can be adjusted.
        //
    
        int chMaster = config->getValueString(ConfigKey("[Soundcard]","ChannelMaster")).toInt();
        int chHead = 0;
        if (config->getValueString(ConfigKey("[Soundcard]","DeviceMaster")) == config->getValueString(ConfigKey("[Soundcard]","DeviceHeadphone")))
            chHead = config->getValueString(ConfigKey("[Soundcard]","ChannelHeadphone")).toInt();

        // Open master device
        if (!open(config->getValueString(ConfigKey("[Soundcard]","DeviceMaster")),
                  config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt(),
                  config->getValueString(ConfigKey("[Soundcard]","Bits")).toInt(),
                  (int)(config->getValueString(ConfigKey("[Soundcard]","LatencyMaster")).toInt()*config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt()/1000.),
                  chMaster,chHead))
        {
            // Reset config data for master and headphone
            config->set(ConfigKey("[Soundcard]","ChannelMaster"),ConfigValue(""));
            config->set(ConfigKey("[Soundcard]","DeviceMaster"),ConfigValue(""));
            config->set(ConfigKey("[Soundcard]","ChannelHeadphone"),ConfigValue(""));
            config->set(ConfigKey("[Soundcard]","DeviceHeadphone"),ConfigValue(""));

            return false;
        }

        // Open headphone if necessary
        if (config->getValueString(ConfigKey("[Soundcard]","DeviceMaster")) != config->getValueString(ConfigKey("[Soundcard]","DeviceHeadphone")) &&
            config->getValueString(ConfigKey("[Soundcard]","ChannelHead")).toInt() > 0)
        {
            // Open slave device
            if (!open(config->getValueString(ConfigKey("[Soundcard]","DeviceMaster")),
                      config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt(),
                      config->getValueString(ConfigKey("[Soundcard]","Bits")).toInt(),
                      (int)(config->getValueString(ConfigKey("[Soundcard]","LatencyHead")).toInt()*config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt()/1000.),
                      0,config->getValueString(ConfigKey("[Soundcard]","ChannelHeadphone")).toInt()))
            {
                // Reset config data for headphone
                config->set(ConfigKey("[Soundcard]","ChannelHeadphone"),ConfigValue(""));
                config->set(ConfigKey("[Soundcard]","DeviceHeadphone"),ConfigValue(""));

                return false;
            }
        }
    }
    
    // Write latency values back to config object
    config->set(ConfigKey("[Soundcard]","LatencyMaster"),ConfigValue((int)((MasterBufferSize*2)/((float)config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt()/1000.))));
    config->set(ConfigKey("[Soundcard]","LatencyHead"),ConfigValue((int)((MasterBufferSize*2*HeadPerMasterBuffer)/((float)config->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt()/1000.))));
    
    return true;
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

void Player::setReader(EngineObject *_reader)
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

  CSAMPLE *p1;

  // Resample; the linear interpolation is done in readfile:
  p1 = reader->process(0, MasterBufferSize);

  // Convert the signal back to SAMPLE and write to the sound cards buffer:
  if (bufferIdx>HeadPerMasterBuffer*3)
      bufferIdx = 0;
  else
      bufferIdx++;
  out_buffer_offset = out_buffer + (MasterBufferSize*2*bufferIdx);
  for (int i=0; i<MasterBufferSize*2; i++)
      out_buffer_offset[i] = (SAMPLE)p1[i];

  return 0;
}

QPtrList<Player::Info> *Player::getInfo()
{
    return &devices;
}

