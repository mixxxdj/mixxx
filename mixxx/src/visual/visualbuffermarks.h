/***************************************************************************
                          visualbuffermarks.h  -  description
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

#ifndef VISUALBUFFERMARKS_H
#define VISUALBUFFERMARKS_H

#include "visualbuffer.h"

class ControlObjectThreadMain;

/**
  *@author Tue & Ken Haste Andersen
  */

class VisualBufferMarks : public VisualBuffer  {
    Q_OBJECT
public:
    VisualBufferMarks(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group);
    ~VisualBufferMarks();
    void update(int iPos, int iLen, long int, int);
    void draw(GLfloat *p, int iLen, float xscale);

    void setMaterialCue(Material *pMaterial);

protected:
    Material *m_pMaterialCue; ///< The material of the cue points.

protected slots:
    void slotUpdateCuePoint(double);

private:
    /** Pointer to cue point position */
    ControlObjectThreadMain *m_pCuePoint;
    /** Index in visual buffer where cue point is set. Is -1 if the cue point
     *  is currently not visible */
    int m_iCuePosition;
};

#endif
