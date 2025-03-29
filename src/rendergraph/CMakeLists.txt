find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
message(STATUS "Qt version ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")

if(QT_VERSION_MINOR GREATER_EQUAL 6)
  set(USE_QSHADER_FOR_GL ON)
endif()

set(
  COMMON_RENDERGRAPH_FILES
  ../common/rendergraph/attributeinit.h
  ../common/rendergraph/attributeset.h
  ../common/rendergraph/geometry.h
  ../common/rendergraph/geometrynode.h
  ../common/rendergraph/material.h
  ../common/rendergraph/material/endoftrackmaterial.cpp
  ../common/rendergraph/material/endoftrackmaterial.h
  ../common/rendergraph/material/patternmaterial.cpp
  ../common/rendergraph/material/patternmaterial.h
  ../common/rendergraph/material/rgbamaterial.cpp
  ../common/rendergraph/material/rgbamaterial.h
  ../common/rendergraph/material/rgbmaterial.cpp
  ../common/rendergraph/material/rgbmaterial.h
  ../common/rendergraph/material/texturematerial.cpp
  ../common/rendergraph/material/texturematerial.h
  ../common/rendergraph/material/unicolormaterial.cpp
  ../common/rendergraph/material/unicolormaterial.h
  ../common/rendergraph/materialshader.h
  ../common/rendergraph/materialtype.h
  ../common/rendergraph/node.h
  ../common/rendergraph/opacitynode.h
  ../common/rendergraph/texture.h
  ../common/rendergraph/types.h
  ../common/rendergraph/uniform.h
  ../common/rendergraph/uniformscache.cpp
  ../common/rendergraph/uniformscache.h
  ../common/rendergraph/uniformset.cpp
  ../common/rendergraph/uniformset.h
  ../common/types.cpp
)

add_subdirectory(opengl)
if(QML)
  add_subdirectory(scenegraph)
endif()
add_subdirectory(shaders)
