add_library(rendergraph_examples
    endoftrackmaterial.cpp
    endoftrackmaterial.h
    texturematerial.cpp
    texturematerial.h
    examplenodes.cpp
    examplenodes.h
)

target_link_libraries(rendergraph_examples PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
)

target_include_directories(rendergraph_examples PUBLIC ../../../ .)
