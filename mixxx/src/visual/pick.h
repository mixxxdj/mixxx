#ifndef __PICKING_INCLUDED__
#define __PICKING_INCLUDED__
#if defined(WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>

/**
 * Forward Declaration.
 */
class CVisualController;

#define BUFSIZE 512

/**
 * Picking Support Class.
 * This class can be used to pick objects.
 *
 * You use it by invoking the method pick(...), which
 * return a unique index of a pickable object.
 */
class CPicking
{
public:

  CPicking();
  void init(CVisualController * controller);

private:

  void processHits(GLint hits, GLuint buffer[]);
  GLuint inFront(GLint hits, GLuint buffer[]);

public:

  int pick(int x,int y);

private:

    GLuint selectBuf[BUFSIZE];
    GLint i,hits;
    GLint viewport[4];
    GLuint    j, num_names, picked;
    GLuint*   p;
    GLboolean save;
    GLuint    depth;

    CVisualController * controller;

};/* End class CPicking */
#endif //__PICKING_INCLUDED__

