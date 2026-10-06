if(LIBREMIDI_NO_WARNINGS)
  return()
endif()

if(MSVC)
  target_compile_options(libremidi PRIVATE
      /W4
      /wd4068 # pragma GCC unrecognized
      /wd4251 # DLL linkage and a public member does not have dll linkage
      /wd4275 # DLL linkage when inheriting from std::runtime_error
      # Too many... $<$<BOOL:${LIBREMIDI_CI}>:/WX>
  )
else()
  target_compile_options(libremidi PRIVATE
      -Wall
      -Wextra
      $<$<BOOL:${LIBREMIDI_CI}>:-Werror>
      $<$<BOOL:${LIBREMIDI_CXX_HAS_WERROR_RETURN_TYPE}>:-Werror=return-type>
  )
endif()
