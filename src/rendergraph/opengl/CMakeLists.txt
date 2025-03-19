add_library(
  rendergraph_gl
  STATIC
  ${COMMON_RENDERGRAPH_FILES}
  attributeset.cpp
  backend/baseattributeset.cpp
  backend/baseattributeset.h
  backend/basegeometry.cpp
  backend/basegeometry.h
  backend/basegeometrynode.cpp
  backend/basegeometrynode.h
  backend/basematerial.cpp
  backend/basematerial.h
  backend/basematerialshader.cpp
  backend/basematerialtype.h
  backend/basenode.h
  backend/basenode.cpp
  backend/baseopacitynode.h
  backend/baseopenglnode.cpp
  backend/baseopenglnode.h
  backend/basetexture.h
  backend/shadercache.h
  engine.cpp
  geometry.cpp
  geometrynode.cpp
  material.cpp
  materialshader.cpp
  rendergraph/context.h
  rendergraph/engine.h
  rendergraph/openglnode.h
  texture.cpp
)

target_link_libraries(rendergraph_gl PUBLIC Qt6::Core Qt6::Gui Qt6::OpenGL)
find_package(Microsoft.GSL CONFIG)
if(Microsoft.GSL_FOUND)
  target_link_libraries(rendergraph_gl PUBLIC Microsoft.GSL::GSL)
else()
  # check if the headers have been installed without cmake config (< 3.1.0)
  check_include_file_cxx(gsl/gsl HAVE_GSL_GSL)
  if(NOT HAVE_GSL_GSL)
    unset(HAVE_GSL_GSL CACHE) # unset cache to re-evaluate this until it succeeds. check_include_file_cxx() has no REQUIRED flag.
    message(FATAL_ERROR "ms-gsl development headers (libmsgsl-dev) not found")
  endif()
endif()
target_compile_definitions(rendergraph_gl PRIVATE rendergraph=rendergraph_gl)

# USE_QSHADER_FOR_GL is set in rendergraph/CMakeLists.txt
if(USE_QSHADER_FOR_GL)
  message(STATUS "Using QShader to load qsb shaders for opengl")
  target_link_libraries(rendergraph_gl PUBLIC Qt6::GuiPrivate)
  target_compile_definitions(rendergraph_gl PRIVATE USE_QSHADER_FOR_GL)
endif()

target_include_directories(rendergraph_gl PUBLIC . ../common)
