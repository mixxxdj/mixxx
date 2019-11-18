cmake_minimum_required(VERSION 2.8.3)
project(layoutstool)

# Setup CMake
set(CMAKE_AUTOMOC  ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ldl -lpthread")

# Setup source files
set(SOURCE_FILES main.cpp layoutstoolmain.cpp layoutstoolmain.h layoutsfilehandler.cpp layoutsfilehandler.h layout.cpp layout.h defs.h utils.cpp utils.h)
add_executable(layoutstool ${SOURCE_FILES})

# X11
find_package(X11)
include_directories(${X11_INCLUDE_DIR})
target_link_libraries(layoutstool ${X11_LIBRARIES})

# Qt4
find_package(Qt4 4.4.3 REQUIRED QtGui)
target_link_libraries(layoutstool Qt4::QtGui)

# Add DL libs
target_link_libraries(layoutstool ${CMAKE_DL_LIBS})

# Install to sibiling bin folder 
install(TARGETS layoutstool DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin)
