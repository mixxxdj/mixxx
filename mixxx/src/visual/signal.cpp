#include "signal.h"
#include "fastvertexarray.h"


/**
 * Default Constructor
 */
CVisualSignal::CVisualSignal(GLfloat *_buffer, CFastVertexArray *_vertex)
{
    buffer = _buffer;
	vertex = _vertex;
    
    ox = oy = oz = 0;
    angle = 0;
    rx = 1;ry=0;rz=0;

    bufferInfo.len1 = 0;
    bufferInfo.len2 = 0;
    length = 0;
};

/**
 * Deconstructor.
 */
CVisualSignal::~CVisualSignal(){};

/**
 * Specialized Drawing Method.
 *
 * @param mode    The rendering mode.
 */
void CVisualSignal::draw(GLenum mode)
{
    CVisualObject::draw(mode);
};

/**
 * Draw Visual Signal.
 * Note that the update method should have been invoked
 * before to this method. If not the signal will not
 * change!!!
 */
void CVisualSignal::draw(){

    glDisable(GL_BLEND);
    //--- matrix mode must be  GL_MODEL
    glPushMatrix();
    glTranslatef(ox,oy,oz);
    float angle=0;
    if(angle!=0)
        glRotatef(angle,rx,ry,rz);
    float yscale = height/2.0f;

    //cout << "len1 " << bufferInfo.len1 << ", len2 " << bufferInfo.len2  << "\n";
    float xscale = length/(bufferInfo.len1+bufferInfo.len2-1);
    glScalef(xscale,yscale,1);

    //--- Now draw the mother fucker:-)

    if (bufferInfo.len1>0)
    {
        glTranslatef(-bufferInfo.p1[0],0,0);
        vertex->draw(bufferInfo.p1,bufferInfo.len1);
    }

    if (bufferInfo.len2>0)
    {
        if (bufferInfo.len1==0)
            glTranslatef(-bufferInfo.p2[0],0,0);
        else
            glTranslatef(bufferInfo.p1[0]+bufferInfo.len1,0,0);
        vertex->draw(bufferInfo.p2, bufferInfo.len2);
    }
  
    //--- Clean up after us
    glPopMatrix();
	glEnable(GL_BLEND);
};

/**
 * Set Signal Origo.
 */
void CVisualSignal::setOrigo(float ox, float oy,float oz){
  this->ox = ox;
  this->oy = oy;
  this->oz = oz;
};

/**

 *
 */
void CVisualSignal::setLength(float length){
  this->length = length;
};

/**
 *
 */
void CVisualSignal::setHeight(float height){
  this->height = height;
};

/**
 *
 */
void CVisualSignal::setRotation(float angle, float rx,float ry,float rz){
  this->angle = angle;
  this->rx = rx;
  this->ry = ry;
  this->rz = rz;
};

/**
 *
 */
void CVisualSignal::setVertexArray(bufInfo i)
{
    bufferInfo = i;
    // std::cout << "len1 " << bufferInfo.len1 << ", len2 " << bufferInfo.len2 << "\n";
};



