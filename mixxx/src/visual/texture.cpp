#include "texture.h"
#include <qimage.h>

/**
 * Constructor.
 */
CTexture::CTexture(){
  loaded = 0;
};

/**
 * Deconstructor.
 */
CTexture::~CTexture(){
  unload();
};

/**
 * Enables texturering an loads a bmp-file into this
 * texture.
 *
 * @param filename    The path and name of a 24 bit uncompressed DIB-file.
 * @param wrap        1 indicates that texture mapping is wrapping.
 * @param decal       If 1 the texture is copied to a surface if 0 the texture is combined with the surfaces color.
 *
 * @return            1 if succesfull otherwise 0.
 */
int CTexture::load(char * filename,const int & wrap,const int & decal)
{

	glGenTextures(1,&texture);

validate();
  
  glBindTexture(GL_TEXTURE_2D,texture);

validate();
  if(wrap==1){
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  }else{
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  }
  (*this).decal = decal;
validate();
  
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
validate();

  if(!glIsEnabled(GL_TEXTURE_2D))
    glEnable(GL_TEXTURE_2D);

  QImage buf, tex;
  if (!buf.load(filename))
  {
	  qDebug("Backplane not loaded");
	  return 0;
  }
  tex = QGLWidget::convertToGLFormat(buf); // Flipped 3dbit RGBA

  glTexImage2D(GL_TEXTURE_2D, 0, 3, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
validate();

  loaded = 1;

  return 1;
};

/**
 * Unloads this texture from memory.
 *
 * @return     1 if succesfull otherwise 0.
 */
int CTexture::unload(void){
  if(loaded)
    glDeleteTextures(1,&texture);
  loaded = 0;
  return 1;
};

/**
 * Call this method to use a texture when you are
 * drawing.
 */
void CTexture::use(){
  if(!loaded){
    glDisable(GL_TEXTURE_2D);
    return;
  }

  if(!glIsEnabled(GL_TEXTURE_2D))
    glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D,texture);

  //DECAL -> Overfør textur direkte
  //MODULATE ->Bland textur med objects materiale
  if(decal==1){
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,GL_DECAL);
  }else{
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,GL_MODULATE);
  }
};

/**
 * Explicitly disables texturemapping.
 */
void CTexture::disable(void){
  glDisable(GL_TEXTURE_2D);
};

/**
 * Explicitly enables texturemapping.
 */
void CTexture::enable(void){
  glEnable(GL_TEXTURE_2D);
};


void CTexture::validate()
{
    GLenum errCode = glGetError();
    if(errCode!=GL_NO_ERROR)
    {
        const GLubyte* errmsg = gluErrorString(errCode);
        qDebug("Visuals: %s",errmsg);
    }
}
