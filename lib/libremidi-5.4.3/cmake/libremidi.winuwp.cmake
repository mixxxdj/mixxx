if(LIBREMIDI_NO_WINUWP)
  return()
endif()

if(LIBREMIDI_HAS_WINMIDI)
  set(LIBREMIDI_HAS_WINUWP 1)
  message(STATUS "libremidi: using WinUWP")

  if(MSVC)
    target_compile_options(libremidi ${_public} /EHsc)
  endif()
  target_compile_definitions(libremidi ${_public} LIBREMIDI_WINUWP)
  return()
endif()

if(CPPWINRT_PATH)
  message(STATUS "libremidi: using WinUWP")
  set(LIBREMIDI_HAS_WINUWP 1)

  target_include_directories(libremidi SYSTEM ${_public} "${CPPWINRT_PATH}")
  target_compile_definitions(libremidi ${_public} LIBREMIDI_WINUWP)
  target_link_libraries(libremidi ${_public} RuntimeObject windowsapp)
  # We don't need /ZW option here (support for C++/CX)' as we use C++/WinRT
  if(MSVC)
    target_compile_options(libremidi ${_public} /EHsc)
  endif()
else()
  message(STATUS "libremidi: Failed to find Windows SDK, UWP MIDI backend will not be available")
  return()
endif()

