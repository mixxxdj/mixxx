set(shaders
    endoftrack.vert
    endoftrack.frag
    texture.vert
    texture.frag
)

qt6_add_shaders(rendergraph_sg "shaders-sg"
    BATCHABLE
    PRECOMPILE
    OPTIMIZED
    PREFIX
        /shaders/rendergraph
    FILES
        ${shaders}
)

include(generated_shaders_gl.cmake)

qt_add_resources(rendergraph_gl "shaders-gl"
    PREFIX
        /shaders/rendergraph
    FILES
        ${generated_shaders_gl}
)
