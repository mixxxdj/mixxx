#ifndef _TEXTURE_INCLUDED_
#define _TEXTURE_INCLUDED_
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <qgl.h>

/**
 * A texture.
 * This class represents a texture map. The map
 * corresponds to a stored bmp-file.
 */
class CTexture
{
public:

  CTexture();
  virtual ~CTexture();

public:

  int load(char * filename,const int & wrap,const int & decal);
  int unload(void);
  void use();

  static void disable(void);
  static void enable(void);

protected:

  GLuint texture; ///< A texture object.
  int loaded;     ///< If true then m_texture is a valid texture object
  int decal;      ///< Remembers how the texture should be rendered.
  void validate();

};
#endif //_TEXTURE_INCLUDED_
