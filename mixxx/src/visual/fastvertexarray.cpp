#include "fastvertexarray.h"

#ifdef __NVSDK__
  #include <GL/glext.h>
  #define GLH_EXT_SINGLE_FILE
  #include <glh_extensions.h>
  #include <glh_obs.h>
  #include <glh_glut.h>
#endif

using namespace std;

// Static members
int CFastVertexArray::usedBufCount = 0;

CFastVertexArray::CFastVertexArray()
{
    pointer = 0;
    nv_fence = false;
};

CFastVertexArray::~CFastVertexArray()
{
    if(pointer)
    {
    #ifdef __NVSDK__
        if (nv_fence)
    #ifdef WIN32
            wglFreeMemoryNV(pointer);
    #else
            glXFreeMemoryNV(pointer);
    #endif
        else
            delete pointer;
        pointer = 0;
    #else
        delete pointer;
        pointer = 0;
    #endif
    }
};

void CFastVertexArray::init(int vertices,int bufferCount)
{
    this->bufferCount = bufferCount;
    verticesPerBuffer = vertices;

#ifdef __NVSDK__
    if (glh_init_extensions("GL_ARB_multitexture " "GL_NV_vertex_array_range " "GL_NV_fence "))
        nv_fence = true;
    else
    {
        nv_fence = false;
        qDebug("Visuals: The following extension(s) are missing: %s",glh_get_unsupported_extensions());
        qDebug("Visuals: Reverting to standard OpenGL.");
    }
    validate();
#endif

    pointer = allocate(3*vertices*bufferCount);

#ifdef __NVSDK__
    if (nv_fence)
    {
        glVertexArrayRangeNV(3*vertices*bufferCount*sizeof(GLfloat), pointer);
        validate();
        glEnableClientState(GL_VERTEX_ARRAY_RANGE_NV);
        validate();
        glEnableClientState(GL_VERTEX_ARRAY);
        validate();
    }
#endif
};

GLfloat *CFastVertexArray::getStartPtr(int no)
{
    int r = usedBufCount;
    usedBufCount+=no;
    return &pointer[r*3*verticesPerBuffer];
}


void CFastVertexArray::draw(GLfloat *p, int vlen)
{
    glVertexPointer(3, GL_FLOAT, 0, p);
    if (!nv_fence)
		glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP,0,vlen);
};

GLfloat *CFastVertexArray::allocate(int size)
{
    //---
    //--- Memory is allocated according to the values
    //---
    //---            Read Freq       Write Freq      Priority
    //---  AGP       [0 - 0.25)      [0 - 0.25)      (0.25 - 0.75]
    //--- VIDEO      [0 - 0.25)      [0 - 0.25)      (0.75 -  1]
    //---
    //---
    float priority = 1.0f;
    float readFrequency = 0;
    float writeFrequency = 0;

    float megabytes = (size * sizeof(GL_FLOAT)/1000000.f);

    GLfloat *array = 0;
#ifdef __NVSDK__
    if (nv_fence)
    {
#ifdef WIN32
        array = (GLfloat *)wglAllocateMemoryNV(size * sizeof(GLfloat), readFrequency, writeFrequency, priority);
#else
        array = (GLfloat *)glXAllocateMemoryNV(size * sizeof(GLfloat), readFrequency, writeFrequency, priority);
#endif
        validate();
    }
#endif
    if(array==0)
    {
        qDebug("Visuals: Unable to allocate %f megabytes of fast video memory. Reverting to standard OpenGL.",megabytes);
        nv_fence = false;

        array = new GLfloat[size*sizeof(GLfloat)];
    }
    
    return array;
};

void CFastVertexArray::validate()
{
    GLenum errCode = glGetError();
    if(errCode!=GL_NO_ERROR)
    {
        const GLubyte* errmsg = gluErrorString(errCode);
        qDebug("Visuals: %s",errmsg);
    }
}
