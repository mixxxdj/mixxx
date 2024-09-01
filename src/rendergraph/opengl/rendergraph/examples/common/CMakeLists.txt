add_library(rendergraph_examples
    examplenodes.cpp
    examplenodes.h
)

target_link_libraries(rendergraph_examples PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
)

target_include_directories(rendergraph_examples PUBLIC ../../../ .)
