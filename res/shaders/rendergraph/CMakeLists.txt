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
        /rendergraph_sg/shaders
    FILES
        ${shaders}
)

include(generated_shaders_gl.cmake)

qt_add_resources(rendergraph_gl "shaders-gl"
    PREFIX
        /rendergraph_gl/shaders
    FILES
        ${generated_shaders_gl}
)
