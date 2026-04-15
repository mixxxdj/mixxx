# included from src/rendergraph/CMakeLists.txt

set(
  shaders
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

if(TARGET rendergraph_sg)
  qt6_add_shaders(rendergraph_sg "shaders-qsb"
      BATCHABLE
      PRECOMPILE
      # FIXME qsl relies on 'spirv-opt' to be available in the path, so only installing it via VCPKG is not sufficient
      # See https://doc.qt.io/qt-6/qtshadertools-build.html#invoking-external-tools
      # OPTIMIZED
      PREFIX
          /shaders/rendergraph
      FILES
          ${shaders}
  )
endif()

if(TARGET rendergraph_gl)
  # USE_QSHADER_FOR_GL is set in src/rendergraph/CMakeLists.txt when Qt >= 6.6
  if(USE_QSHADER_FOR_GL)
    # Add the .qsb shader bundles; rendergraph::MaterialShader will use
    # QShader to extract the GLSL shader from the bundle.
    message(STATUS "Adding qsb shaders to rendergraph_gl")
    qt6_add_shaders(rendergraph_gl "shaders-qsb"
          BATCHABLE
          PRECOMPILE
          # FIXME qsl relies on 'spirv-opt' to be available in the path, so only installing it via VCPKG is not sufficient
          # See https://doc.qt.io/qt-6/qtshadertools-build.html#invoking-external-tools
          # OPTIMIZED
          PREFIX
              /shaders/rendergraph
          FILES
              ${shaders}
    )
  else()
    # Use GLSL shaders extracted from the .qsb shader bundles using
    # generate_shaders_gl.pl
    message(STATUS "Adding gl shaders to rendergraph_gl")
    include(generated_shaders_gl.cmake)

    qt_add_resources(rendergraph_gl "shaders-gl"
          PREFIX
              /shaders/rendergraph
          FILES
              ${generated_shaders_gl}
    )
  endif()
endif()
