cmake_minimum_required(VERSION 3.24)
project(midi_gui CXX)

set(LIBREMIDI_NO_WINUWP 0)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../" libremidi-bin)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
add_executable(midi_gui main.cpp)
target_link_libraries(midi_gui
  PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    libremidi::libremidi
)

