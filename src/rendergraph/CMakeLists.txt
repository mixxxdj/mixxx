find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
message(STATUS "Qt version ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")

if(QT_VERSION_MINOR GREATER_EQUAL 6)
  set(USE_QSHADER_FOR_GL ON)
endif()

add_subdirectory(opengl)
add_subdirectory(scenegraph)
add_subdirectory(../../res/shaders/rendergraph shaders)
