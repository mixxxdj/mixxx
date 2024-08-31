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
