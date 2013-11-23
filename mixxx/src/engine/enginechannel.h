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
#include "configobject.h"

class ControlObject;
class EngineBuffer;
class EnginePregain;
class EngineBuffer;
class EngineFilterBlock;
class EngineClipping;
class EngineFlanger;
class EngineVuMeter;
class EngineVinylSoundEmu;
class ControlPushButton;
class ControlObject;

class EngineChannel : public EngineObject {
    Q_OBJECT
  public:
    enum ChannelOrientation {
        LEFT = 0,
        CENTER,
        RIGHT,
    };

    EngineChannel(const char *pGroup, ChannelOrientation defaultOrientation = CENTER);
    virtual ~EngineChannel();

    virtual ChannelOrientation getOrientation();
    virtual const QString& getGroup() const;

    virtual bool isActive() = 0;
    virtual bool isPFL();
    virtual bool isMaster();

    virtual void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize) = 0;

    // TODO(XXX) This hack needs to be removed.
    virtual EngineBuffer* getEngineBuffer() {
        return NULL;
    }

  private:
    const QString m_group;
    ControlPushButton* m_pMaster;
    ControlPushButton* m_pPFL;
    ControlObject* m_pOrientation;
};

#endif
