/***************************************************************************
                          visualbuffertemporal.h  -  description
                             -------------------
    begin                : Tue Aug 31 2004
    copyright            : (C) 2004 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VISUALBUFFERTEMPORAL_H
#define VISUALBUFFERTEMPORAL_H

#include "visualbuffer.h"
#include <qtimer.h>

class ControlObjectThreadMain;

/**
  *@author Tue Haste Andersen
  */

class VisualBufferTemporal : public VisualBuffer  {
    Q_OBJECT
public:
    VisualBufferTemporal(EngineBuffer *pEngineBuffer, const char *group);
    ~VisualBufferTemporal();
    virtual void update(int iPos, int iLen);
    virtual void draw(GLfloat *p, int iLen, float);

private slots:
    void update();
    void slotPlayposChanged();
    void slotTemporalChanged();   
private:
    /** Pointer to phase, period controls, and rate slider */
    ControlObjectThreadMain *m_pControlPhase, *m_pControlShape, *m_pControlBpm, *m_pControlRate, *m_pControlBeatFirst;
    /** Buffer used when drawing around start and end of buffer */
    GLfloat m_fWrapBuffer[12];
    /** Timer used to update temporal buffer */
    QTimer m_qTimer;
    /** Variables used to determine when to update display */
    bool m_bTemporalChanged, m_bPlayposChanged;
    /** Timer variable */
    int m_iTimesCalled;
};

#endif
