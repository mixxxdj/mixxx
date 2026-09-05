if(NOT WIN32)
  return()
endif()

include(libremidi.winmm)
include(libremidi.kdmapi)

if(NOT LIBREMIDI_NO_WINMIDI OR NOT LIBREMIDI_NO_WINUWP)
  include(libremidi.cppwinrt)
  include(libremidi.winmidi)
  include(libremidi.winuwp)
endif()
