cmake_minimum_required(VERSION 3.16)
project(rendergraph LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick ShaderTools)

qt_standard_project_setup()

add_subdirectory(../ rendergraph)
add_subdirectory(shaders)
add_subdirectory(gl_example)
add_subdirectory(sg_example)
add_subdirectory(common)
