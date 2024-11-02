add_library(rendergraph_sg
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

target_link_libraries(rendergraph_sg PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)
if(WIN32)
  find_package(Microsoft.GSL CONFIG)
  if(Microsoft.GSL_FOUND)
    target_link_libraries(rendergraph_sg PRIVATE Microsoft.GSL::GSL)
  else()
    # check if the headers have been installed without cmake config (< 3.1.0)
    check_include_file_cxx(gsl/gsl HAVE_GSL_GSL)
    if(NOT HAVE_GSL_GSL)
      unset(HAVE_GSL_GSL CACHE) # unset cache to re-evaluate this until it succeeds. check_include_file_cxx() has no REQUIRED flag.
      message(FATAL_ERROR "ms-gsl development headers (libmsgsl-dev) not found")
    endif()
  endif()
endif()
target_compile_definitions(rendergraph_sg PRIVATE rendergraph=rendergraph_sg)

target_include_directories(rendergraph_sg PUBLIC . ../common)
