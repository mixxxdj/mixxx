add_library(
  rendergraph_sg
  STATIC
  ${COMMON_RENDERGRAPH_FILES}
  attributeset.cpp
  backend/baseattributeset.cpp
  backend/baseattributeset.h
  backend/basegeometry.h
  backend/basegeometrynode.h
  backend/basematerial.cpp
  backend/basematerial.h
  backend/basematerialshader.cpp
  backend/basematerialtype.h
  backend/basenode.h
  backend/baseopacitynode.h
  backend/basetexture.h
  context.cpp
  geometry.cpp
  geometrynode.cpp
  material.cpp
  materialshader.cpp
  rendergraph/context.h
  texture.cpp
)

target_link_libraries(
  rendergraph_sg
  PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
)
find_package(Microsoft.GSL CONFIG)
if(Microsoft.GSL_FOUND)
  target_link_libraries(rendergraph_sg PUBLIC Microsoft.GSL::GSL)
else()
  # check if the headers have been installed without cmake config (< 3.1.0)
  check_include_file_cxx(gsl/gsl HAVE_GSL_GSL)
  if(NOT HAVE_GSL_GSL)
    unset(HAVE_GSL_GSL CACHE) # unset cache to re-evaluate this until it succeeds. check_include_file_cxx() has no REQUIRED flag.
    message(FATAL_ERROR "ms-gsl development headers (libmsgsl-dev) not found")
  endif()
endif()
target_compile_definitions(rendergraph_sg PRIVATE rendergraph=rendergraph_sg)

target_include_directories(rendergraph_sg PUBLIC . ../common)
