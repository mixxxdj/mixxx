/***************************************************************************
                          fastvertexarray.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen and Kenny 
                                       Erleben
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

#ifndef FASTVERTEXARRAY_H
#define FASTVERTEXARRAY_H

#include <iostream>
#include "signalvertexbuffer.h"

#include <qgl.h>

/**
 * Fast Vertex Array.
 * This class implements a vertex array resident
 * in the video memeory.
 *
 * Note: There are some caveats when reading data from the vertex
 * array, according to the white papers, the memory used by the
 * vertex array is uncached and therefore slow to read from.
 */
class FastVertexArray
{
public:

  /**
   * Default Constructor.
   */
  FastVertexArray();
  /**
   * Deconstructor.
   */
  virtual ~FastVertexArray();

  /**
   * Initialize Vertex Array.
   *
   * Developers Note: It seems that there is
   * a upper limit on the size of data that can
   * be within a fence. Measurements show that
   * around 200000 bytes the drawing looks akward???
   *
   *
   *
   *
   * @param vertices            The number of vertices
   * @param bufferCount         The number of wanted buffers
   */
  void init(int vertices,int bufferCount);
  
  GLfloat *getStartPtr(int no);

  /**
   * Draw Vertex Array.
   */
  void draw(GLfloat *p, int _vlen);


private:

  /**
   * Update Vertex Array.
   * This method was intended for updating information
   * in a subpart of the vertex array, corresponding to
   * a single buffer.
   *
   * @param pointer   A pointer to the vertex array of the buffer.
   * @param size      The size (#floats) of the vertex array.
   */
//  void update(float *p, int _vlen, GLfloat *signal, int i);

  /**
   * Memory Allocation Routine.
   *
   * @param size    The size (#floats) of the vertex array
   *
   * @return        A pointer to the allocated vertex array.
   */
  GLfloat *allocate(int size);

  /**
   * Auxiliary Method.
   * This is used to validate the error state of openGL.
   */
  void validate();

private:

  bool nv_fence;    // Nvidia fence support
  static int usedBufCount; ///< Count of buffers already in use
  int verticesPerBuffer;
  int bufferCount;
  
public:

  GLfloat *pointer; ///<  A pointer to the "entire" vertex array.

};
#endif

