#ifndef _OPENGL_LIGHT_INCLUDED_
#define _OPENGL_LIGHT_INCLUDED_

/**
 * A Light.
 * This class encapsulates an openGL lightsource into an easy
 * to use interface.
 */
class CLight
{
public:

  CLight();
  virtual ~CLight();

public:

  void enable();
  void disable();

public:

  float ambient[4];         ///< The ambient color rgba.
  float diffuse[4];         ///< The diffuse color rgba.
  float specular[4];        ///< The specular color rgba.
  float position[4];        ///< The position of the light source.
  
private:

  static int nextLight;   ///< The next light number.
  int i;                  ///< The light number of this light.

protected:

  int getLightIndex();

};
#endif //_OPENGL_LIGHT_INCLUDED_

