/***************************************************************************
                          enginechannel.h  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
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

#ifndef ENGINECHANNEL_H
#define ENGINECHANNEL_H

#include "engineobject.h"

class EngineBuffer;
class EnginePregain;
class EngineBuffer;
class EngineFilterBlock;
class EngineClipping;
class EngineVolume;
class EngineFlanger;
class EngineVuMeter;
class EngineVinylSoundEmu;
class ControlPushButton;

class EngineChannel : public EngineObject {
public:
    enum ChannelOrientation {
        LEFT = 0,
        CENTER,
        RIGHT,
    };

    EngineChannel(const char *group, ConfigObject<ConfigValue>* pConfig,
                  ChannelOrientation defaultOrientation = CENTER);
    virtual ~EngineChannel();

    bool isPFL();
    ChannelOrientation getOrientation();
    const QString& getGroup();

    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

    // TODO(XXX) This hack needs to be removed.
    EngineBuffer* getEngineBuffer();

private:
    const QString m_group;
    ConfigObject<ConfigValue>* m_pConfig;
    ControlPushButton* m_pPFL;
    ControlObject* m_pOrientation;

    EngineBuffer* m_pBuffer;
    EngineClipping* m_pClipping;
    EngineFilterBlock* m_pFilter;
    EngineFlanger* m_pFlanger;
    EnginePregain* m_pPregain;
    EngineVinylSoundEmu* m_pVinylSoundEmu;
    EngineVolume* m_pVolume;
    EngineVuMeter* m_pVUMeter;
};

#endif
