cmake_minimum_required(VERSION 3.16)
project(rendergraph LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick ShaderTools)

if(NOT(QT_VERSION VERSION_LESS "6.3.0"))
 qt_standard_project_setup()
endif()

add_subdirectory(../ rendergraph)

target_compile_definitions(rendergraph_gl PUBLIC
  $<$<CONFIG:Debug>:MIXXX_DEBUG_ASSERTIONS_ENABLED>
)
target_compile_definitions(rendergraph_sg PUBLIC
  $<$<CONFIG:Debug>:MIXXX_DEBUG_ASSERTIONS_ENABLED>
)

add_subdirectory(gl_example)
add_subdirectory(sg_example)
