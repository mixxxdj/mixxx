if(LIBREMIDI_NO_WINMM)
  return()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES WindowsStore)
  return()
endif()

message(STATUS "libremidi: using WinMM")
set(LIBREMIDI_HAS_WINMM 1)
target_compile_definitions(libremidi
  ${_public}
    LIBREMIDI_WINMM
    UNICODE=1
    _UNICODE=1
)
target_link_libraries(libremidi ${_public} winmm)
