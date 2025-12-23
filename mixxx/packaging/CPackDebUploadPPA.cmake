# This file is executed during cpack time.
# Possible commands are
# cpack -G External -D DEB_UPLOAD_PPA=ppa:xxxxx/xxxxxx
# cpack -G External -D DEB_SOURCEPKG=true
# cpack -G External -D DEB_BUILD=true

find_program(CPACK_DEBIAN_DEBUILD debuild)
if(NOT CPACK_DEBIAN_DEBUILD)
  message(
    FATAL_ERROR
    "debuild not found, required for cpack -G External -D DEB_UPLOAD_PPA=true"
  )
endif()

if(DEB_UPLOAD_PPA)
  find_program(CPACK_DEBIAN_DPUT dput)
  if(NOT CPACK_DEBIAN_DPUT)
    message(
      FATAL_ERROR
      "dput not found, required for cpack -G External -D DEB_UPLOAD_PPA=true"
    )
  endif()
endif()

find_program(CPACK_DEBIAN_DEBCHANGE debchange)
if(NOT CPACK_DEBIAN_DEBCHANGE)
  message(
    FATAL_ERROR
    "debchange not found, required for cpack -G External -D DEB_UPLOAD_PPA=true"
  )
endif()

find_program(CPACK_DEBIAN_MARKDOWN markdown)
if(NOT CPACK_DEBIAN_MARKDOWN)
  message(
    FATAL_ERROR
    "markdown not found, required for cpack -G External -D DEB_UPLOAD_PPA=true"
  )
endif()

find_program(CPACK_DEBIAN_DOCBOOK_TO_MAN docbook-to-man)
if(NOT CPACK_DEBIAN_DOCBOOK_TO_MAN)
  message(
    FATAL_ERROR
    "docbook-to-man not found, required for cpack -G External -D DEB_UPLOAD_PPA=true"
  )
endif()

# PR branches have no access to the pgp key. Don't sign.
find_program(CPACK_DEBIAN_GPG gpg)
if(CPACK_DEBIAN_GPG)
  execute_process(
    COMMAND ${CPACK_DEBIAN_GPG} --fingerprint "${CPACK_PACKAGE_CONTACT}"
    RESULT_VARIABLE CPACK_DEBIAN_GPG_RET
  )
endif()
if(NOT CPACK_DEBIAN_GPG_RET EQUAL "0")
  message(
    WARNING
    "No secret key found for \"${CPACK_PACKAGE_CONTACT}\", skip signing"
  )
  set(CPACK_DEBIAN_DEBUILD_NOSIGN "--no-sign")
endif()

message(NOTICE "Creating mixxx_${CPACK_DEBIAN_PACKAGE_VERSION}.orig.tar.gz")
execute_process(
  COMMAND
    tar -czf "mixxx_${CPACK_DEBIAN_PACKAGE_VERSION}.orig.tar.gz"
    ${CPACK_PACKAGE_FILE_NAME}
  WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}
)

# Save Git info from original working tree to allow building without git
# uses GIT_BRANCH GIT_DESCRIBE GIT_COMMIT_DATE GIT_COMMIT_YEAR GIT_COMMIT_COUNT GIT_DIRTY
configure_file(
  ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/src/gitinfo.h.in
  ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/src/gitinfo.h.in
  @ONLY
)

message(NOTICE "Creating debian folder")
file(
  COPY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/packaging/debian
  DESTINATION ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
)
file(
  REMOVE
  ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/control.in
)

execute_process(
  COMMAND
    ${CPACK_DEBIAN_MARKDOWN}
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/CHANGELOG.md
  OUTPUT_FILE NEWS.html
  WORKING_DIRECTORY
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian
)

execute_process(
  COMMAND ${CPACK_DEBIAN_DOCBOOK_TO_MAN} debian/mixxx.sgml
  OUTPUT_FILE mixxx.1
  WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
)

file(
  COPY
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/res/linux/mixxx-usb-uaccess.rules
  DESTINATION ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian
)
file(
  RENAME
  ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/mixxx-usb-uaccess.rules
  ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/mixxx.mixxx-usb-uaccess.udev
)

if(DEB_BUILD)
  execute_process(
    COMMAND lsb_release --short --codename
    OUTPUT_VARIABLE BUILD_MACHINE_RELEASE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

foreach(release ${CPACK_DEBIAN_DISTRIBUTION_RELEASES})
  if(release STREQUAL "jammy")
    set(CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS_EXTRA "libqt6shadertools6-dev,")
  else()
    set(CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS_EXTRA "qt6-shadertools-dev,")
  endif()

  configure_file(
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/packaging/debian/control.in
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/control
    @ONLY
  )

  file(
    COPY
      ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/packaging/debian/changelog
    DESTINATION ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian
  )
  execute_process(
    COMMAND
      ${CPACK_DEBIAN_DEBCHANGE} -v
      "${CPACK_DEBIAN_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}~${release}"
      -M "Build of ${CPACK_DEBIAN_PACKAGE_VERSION}"
    WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
  )
  execute_process(
    COMMAND
      ${CPACK_DEBIAN_DEBCHANGE} -r -D ${release} -M
      "Build of ${CPACK_DEBIAN_PACKAGE_VERSION}"
    WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
  )

  if(DEB_UPLOAD_PPA OR DEB_SOURCEPKG)
    execute_process(
      COMMAND ${CPACK_DEBIAN_DEBUILD} -S -sa -d ${CPACK_DEBIAN_DEBUILD_NOSIGN}
      WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
      RESULT_VARIABLE CPACK_DEBIAN_DEBUILD_RET
    )
    if(NOT CPACK_DEBIAN_DEBUILD_RET EQUAL "0")
      message(
        FATAL_ERROR
        "${CPACK_DEBIAN_DEBUILD} returned exit code ${CPACK_DEBIAN_DEBUILD_RET}"
      )
    endif()
  endif()
  if(BUILD_MACHINE_RELEASE STREQUAL release AND DEB_BUILD)
    execute_process(
      COMMAND ${CPACK_DEBIAN_DEBUILD} -b ${CPACK_DEBIAN_DEBUILD_NOSIGN}
      WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
    )
  endif()
  if(DEB_UPLOAD_PPA)
    execute_process(
      COMMAND
        ${CPACK_DEBIAN_DPUT} ${DEB_UPLOAD_PPA}
        "mixxx_${CPACK_DEBIAN_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}~${release}_source.changes"
      WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}
    )
  endif()
endforeach()

if(DEB_SOURCEPKG OR DEB_BUILD)
  file(
    GLOB ARTIFACTS
    "${CPACK_TOPLEVEL_DIRECTORY}/mixxx_${CPACK_DEBIAN_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}*"
    "${CPACK_TOPLEVEL_DIRECTORY}/mixxx-dbgsym_${CPACK_DEBIAN_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_RELEASE}*"
  )
  file(COPY ${ARTIFACTS} DESTINATION ${CPACK_PACKAGE_DIRECTORY})
endif()
