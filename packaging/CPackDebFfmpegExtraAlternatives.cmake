# CPack post build script for the DEB generator.
#
# Debian and Ubuntu ship FFmpeg twice: the regular libraries (libavcodec60, ...)
# and the "-extra" builds with more codecs (libavcodec-extra60, ...). The extra
# packages Provide the regular names but also Conflict with them. Distributions
# such as Linux Mint install the extra builds by default through
# mint-meta-codecs.
#
# CPack's shlibdeps only writes the regular names, e.g.
#   libavcodec60 (>= 7:6.0)
# apt accepts the versioned Provides of the extra package, but GDebi and the
# graphical package installers of Mint and Ubuntu do not: they want the regular
# library, which removes libavcodec-extra60, libavcodec-extra and
# mint-meta-codecs (and whatever depends on them). Naming both variants as
# alternatives keeps the codecs the user already has:
#   libavcodec60 (>= 7:6.0) | libavcodec-extra60 (>= 7:6.0)

set(_mixxxFfmpegLibraries avcodec avformat avfilter)

foreach(_package IN LISTS CPACK_PACKAGE_FILES)
  if(NOT _package MATCHES "\\.deb$")
    continue()
  endif()

  get_filename_component(_packageDir "${_package}" DIRECTORY)
  set(_unpackDir "${_packageDir}/_deb_extra_alternatives")
  file(REMOVE_RECURSE "${_unpackDir}")

  execute_process(
    COMMAND dpkg-deb --raw-extract "${_package}" "${_unpackDir}"
    RESULT_VARIABLE _result
    ERROR_VARIABLE _error
  )
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Could not unpack ${_package}: ${_error}")
  endif()

  set(_controlFile "${_unpackDir}/DEBIAN/control")
  file(READ "${_controlFile}" _control)
  set(_originalControl "${_control}")

  foreach(_library IN LISTS _mixxxFfmpegLibraries)
    # Only rewrite plain entries such as "libavcodec60 (>= 7:6.0)" that are
    # followed by a separator, so already rewritten alternatives and the
    # "-extra" names themselves are left alone.
    string(
      REGEX REPLACE
      "(: |, )lib${_library}([0-9]+)( \\([^)]*\\))?(,|\n)"
      "\\1lib${_library}\\2\\3 | lib${_library}-extra\\2\\3\\4"
      _control
      "${_control}"
    )
  endforeach()

  if(_control STREQUAL _originalControl)
    message(STATUS "No FFmpeg dependency to extend in ${_package}")
    file(REMOVE_RECURSE "${_unpackDir}")
    continue()
  endif()

  file(WRITE "${_controlFile}" "${_control}")
  # dpkg-deb refuses control directories with group/other write permission.
  file(
    CHMOD
    "${_unpackDir}/DEBIAN"
    DIRECTORY_PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      OWNER_EXECUTE
      GROUP_READ
      GROUP_EXECUTE
      WORLD_READ
      WORLD_EXECUTE
  )
  file(
    CHMOD
    "${_controlFile}"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )

  # xz is understood by every dpkg and package installer, while the zstd
  # default of newer dpkg versions is not.
  execute_process(
    COMMAND
      dpkg-deb --root-owner-group -Zxz --build "${_unpackDir}" "${_package}"
    RESULT_VARIABLE _result
    OUTPUT_QUIET
    ERROR_VARIABLE _error
  )
  file(REMOVE_RECURSE "${_unpackDir}")
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Could not repack ${_package}: ${_error}")
  endif()
  message(
    STATUS
    "Allowed the FFmpeg -extra libraries as alternatives in ${_package}"
  )
endforeach()
