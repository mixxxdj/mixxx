/***************************************************************************
                          visualbuffersignal.h  -  description
                             -------------------
    begin                : Fri Jun 13 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#ifndef VISUALBUFFERSIGNAL_H
#define VISUALBUFFERSIGNAL_H

#include "visualbuffer.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class ControlObjectThreadMain;
  
// Sample rate in Hertz at which the sound buffer is resampled
const int kiVisualResampleRate = 1000;
const float kfWaveshapeFactor = 0.99f;

class VisualBufferSignal : public VisualBuffer  
{
    Q_OBJECT
public:
    VisualBufferSignal(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group);
    ~VisualBufferSignal();
    virtual void update(int iPos, int iLen, long int liFileStartPos, int iBufferStartPos);
    virtual void draw(GLfloat *p, int iLen, float);

public slots:
    void slotUpdateBpm(double v);
    void slotUpdateBeatFirst(double v);

private:
    /** Pointer to control object holding beat info */
    ControlObjectThreadMain *m_pControlBpm, *m_pControlBeatFirst;
    /** Local copies of beat info */
    double m_dBeatFirst, m_dBpm, m_dBeatDistance;
    /** Used in low pass filtering */
    float m_fLastPositive, m_fLastNegative;
    /** Buffer used when drawing around start and end of buffer */
    GLfloat m_fWrapBuffer[12], m_fWrapBuffer2[12];
    /** Pointer to the secondary buffer, an openGL vertex array */
    GLfloat *m_pBuffer2;
    double m_dAbsStartpos;
    /** Absoulte start file position */
    long int m_liFileStartPos;
    /** Buffer start position */
    int m_iBufferStartPos;    
};

#endif
