/***************************************************************************
                          visualbuffer.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen and Kenny
                                       Erleben
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is fr ee software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VISUALBUFFER_H
#define VISUALBUFFER_H

/** Display rate in Hz. This relates to the display resolution */
const int MAXDISPLAYRATE = 100;

/**
  * A Signal Vertex Array Buffer.
  * This class is capable of preprosseing a
  * signal into a 3D representation and plotting it.
  *
  * Each signal value is transformed into a 3D
  * point. The signal is plotted using the draw method, usually invoked from
  * a VisualDisplayBuffer class.
  */

#include "../defs.h"
#include <qobject.h>
#include <qgl.h>
#include "material.h"

class ReaderExtract;
class EngineBuffer;
class ControlObject;

typedef struct
{
    float           *p1, *p2;
    int             len1, len2;
    float           corr;
} bufInfo;

class VisualBuffer : public QObject
{
    Q_OBJECT
public:
    VisualBuffer(EngineBuffer *pEngineBuffer, const char *group);
    VisualBuffer(ReaderExtract *pReaderExtract, EngineBuffer *pEngineBuffer, const char *group);
    virtual ~VisualBuffer();
    bool eventFilter(QObject *o, QEvent *e);
    /** Update and resample signal vertex buffer */
    virtual void update(int iPos, int iLen) = 0;
    /** Draw the buffer */
    virtual void draw(GLfloat *p, int iLen, float xscale) = 0;

    bufInfo getVertexArray();
    int getBufferLength();
    int getDisplayLength();
    GLfloat *getBasePtr();
    void setColorFg(float r, float g, float b);
    void setColorBg(float r, float g, float b);

public slots:
    /** Update resample factor based on information from ReaderExtract object */
    void slotSetupBuffer(int bufferlen, int srate);

protected:
    /** This is used to validate the error state of openGL. */
    void validate();
    /** Pointer to rate control object */
    ControlObject *m_pRate;
    /** Pointer to the actual buffer, an openGL vertex array */
    GLfloat *m_pBuffer;
    /** Pointer to source buffer from ReaderExtract */
    CSAMPLE *m_pSource;
    /** The total number of samples in the buffer m_pBuffer. */
    int m_iLen;
    /** The total number of samples in the buffer m_pSource. */
    int m_iSourceLen;
    /** Resample factor determining the factor to reduce the incomming signal with */
    CSAMPLE m_fResampleFactor;
    /** Display rate. Rate at which this buffer represents. The x axis values of this buffer correspond to samples
     *       * at MAXDISPLAYRATE positions */
    CSAMPLE m_fDisplayRate;
    /** Factor between this buffers sample rate, and MAXDISPLAYRATE */
    CSAMPLE m_fDisplayFactor;

     /** Factor used in convesion of position and length between ReaderExtractWave and
      * the associated ReaderExtract position and length*/
    CSAMPLE m_fReaderExtractFactor;
    /** Number of samples of this buffer to display (used in getVertexArray) */
    int m_iDisplayLen;
    /** Pounter to ReaderExtract */
    ReaderExtract *m_pReaderExtract;
    /** Pointer to EngineBuffer */
    EngineBuffer *m_pEngineBuffer;
    /** Material */
    Material m_materialFg, m_materialBg;
    /** Buffer and absulute play position, read from EngineBuffer */
    double m_dBufferPlaypos, m_dAbsPlaypos;


};
#endif



