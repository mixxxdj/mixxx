# rendergraph shaders

The CMakeLists.txt in this folder generates qsb shader bundles from spirv shaders, to
be used by `rendergraph::MaterialShader`.

## For use with QML / Qt scene graph

The qsb files can be used directly through `QSGShader`. This includes the scenegraph
implementation of rendergraph.

## For use with OpenGL

Depending on the Qt version, the opengl implementation of `rendergraph::MaterialShader`
uses either the .qsb shader bundles directly, or the extracted GLSL shaders:

### Qt >= 6.6

The GLSL shaders are extracted programmatically with `QShader` and then used with
`QOpenGLShader`.

### Qt < 6.6

The GLSL shader have to extracted from the qsb shader bundles to be used by `QOpenGLShader`.
This can be done using the script `rg_generate_shaders_gl.py` in the mixxx/tools directory:

```console
$ ../../../tools/rg_generate_shaders_gl.py --cmake generated_shaders_gl.cmake *.vert *.frag
```

To use this script, make sure that the qsb and spirv commands are in your path. qsb is part
of Qt. spirv is part of the Vulkan SDK and can be downloaded from <https://vulkan.org>

The script also generates the file `generated_shaders_gl.cmake` which sets a cmake
variable containing a list of all GLSL shaders, used by the CMakeLists.txt in this folder.
