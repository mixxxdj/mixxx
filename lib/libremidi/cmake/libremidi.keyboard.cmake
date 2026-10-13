if(LIBREMIDI_NO_KEYBOARD)
  return()
endif()

set(LIBREMIDI_HAS_KEYBOARD 1)

target_compile_definitions(libremidi
  ${_public}
    LIBREMIDI_KEYBOARD
)
