#ifndef _OPENGL_MATERIAL_INCLUDED_
#define _OPENGL_MATERIAL_INCLUDED_

/**
 * A material.
 * This class encapsulates an openGL material into an easy to use interface.
 */
class CMaterial
{
public:

  CMaterial();
  virtual ~CMaterial();

public:

  void use();

public:

  float ambient[4];   ///< The ambient color rgba.
  float diffuse[4];   ///< The diffuse color rgba.
  float specular[4];  ///< The specular color rgba.
  float emission[4];  ///< The emissive color rgba.
  float shininess;    ///< The shininess of this material 0..180.

};
#endif // _OPENGL_MATERIAL_INCLUDED_

