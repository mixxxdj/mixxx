/***************************************************************************
                          enginebuffercue.h  -  description
                             -------------------
    copyright            : (C) 2005 by Tue Haste Andersen
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

#ifndef ENGINEBUFFERCUE_H
#define ENGINEBUFFERCUE_H

#include "engineobject.h"
#include "controlobjectthreadmain.h"

class ControlObject;
class ControlObjectThreadMain;
class ControlPushButton;
class EngineBuffer;

class EngineBufferCue : public EngineObject 
{
    Q_OBJECT
public:
    EngineBufferCue(const char *group, EngineBuffer *pEngineBuffer);
    ~EngineBufferCue();
    void process(const CSAMPLE *, const CSAMPLE *, const int);

public slots:
    void slotControlCueGoto(double=0);
    void slotControlCueGotoAndStop(double=0);
    void slotControlCueSet(double=0);
    void slotControlCuePreview(double);
    void slotControlCueSimple(double);
    void slotControlPlay(double);
    void slotControlCueDefault(double v);
    void slotControlCueCDJ(double v);

private:
    void saveCuePoint(double cue);
    
    /** Controls used to manipulate the cue point */
    ControlPushButton *buttonCueSet, *buttonCueGoto, *buttonCueGotoAndStop, *buttonCuePreview, *buttonCueCDJ;
    /** A single control to manipulate the cue point (suggested by Thread) */
    ControlPushButton *buttonCueSimple;
    /** Default cue control */
    ControlPushButton *buttonCueDefault;
    /** Cue behavior preference */
    ControlObject *m_pControlCueDefault;
    /** Pointer to play button */
    ControlObject *playButton;
    /** Storage of cue point */
    ControlObject *cuePoint;     
    /** Is true if currently in cue preview mode. We need to keep track of the state
      * because the preview key slot can be activated many times during one preview */
    bool m_bCuePreview;
    /** Pointer to EngineBuffer */
    EngineBuffer *m_pEngineBuffer;
    /** Group identifier string */
    const char* m_group;
};

#endif
