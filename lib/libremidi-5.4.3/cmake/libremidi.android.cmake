### Android ###
if(ANDROID AND NOT LIBREMIDI_NO_ANDROID)
  # Check for AMidi API availability (API level 29+)
  # Also we need API 31+ for JNI_GetCreatedJavaVMs
  if(ANDROID_NATIVE_API_LEVEL AND ANDROID_NATIVE_API_LEVEL GREATER_EQUAL 31)
    message(STATUS "libremidi: Using Android AMidi API")
    message(STATUS "       !!! Remember to include MidiDeviceCallback.java in your Android app")
    message(STATUS "       !!! See the example in examples/android/java/dev/celtera/libremidi")

    target_compile_definitions(libremidi ${_public} LIBREMIDI_ANDROID=1)
    target_link_libraries(libremidi ${_public} amidi log nativehelper)
    target_sources(libremidi ${_public}
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/libremidi/backends/android/helpers.cpp>"
    )
  else()
    message(FATAL_ERROR "libremidi: Android AMidi API requires API level 31+ (current: ${ANDROID_NATIVE_API_LEVEL})")
  endif()
endif()
