qt_add_shaders(mixxx-lib "waveform_shaders"
  PREFIX "/shaders"
  FILES
      "rgbsignal_qml.frag"
)

# Workaround for https://bugreports.qt.io/browse/QTBUG-118500 that can be
# removed once Qt fixes the shader target generation for Xcode.
# See also https://github.com/mixxxdj/mixxx/issues/13378
if(CMAKE_GENERATOR STREQUAL "Xcode")
  add_dependencies(mixxx-lib_resources_1 mixxx-lib_waveform_shaders)
endif()
