set(shaders
    endoftrack.frag
    endoftrack.vert
    pattern.frag
    pattern.vert
    rgb.frag
    rgb.vert
    rgba.frag
    rgba.vert
    texture.frag
    texture.vert
    unicolor.frag
    unicolor.vert
)

qt6_add_shaders(rendergraph_sg "shaders-qsb"
    BATCHABLE
    PRECOMPILE
    OPTIMIZED
    PREFIX
        /shaders/rendergraph
    FILES
        ${shaders}
)

if(USE_QSHADER_FOR_GL)
    message(STATUS "Adding qsb shaders to rendergraph_gl")
    qt6_add_shaders(rendergraph_gl "shaders-qsb"
        BATCHABLE
        PRECOMPILE
        OPTIMIZED
        PREFIX
            /shaders/rendergraph
        FILES
            ${shaders}
    )
else()
    message(STATUS "Adding gl shaders to rendergraph_gl")
    include(generated_shaders_gl.cmake)

    qt_add_resources(rendergraph_gl "shaders-gl"
        PREFIX
            /shaders/rendergraph
        FILES
            ${generated_shaders_gl}
    )
endif()
