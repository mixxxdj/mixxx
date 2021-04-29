# This file is executed during cpack time.
# The command is
# cpack -G DEB

find_program( CPACK_DEBIAN_DEBHELPER dh_prep )
if( NOT CPACK_DEBIAN_DEBHELPER )
    message( FATAL_ERROR "debhelper not found, required for cpack -G DEB" )
endif()

find_program( CPACK_DEBIAN_MARKDOWN markdown )
if( NOT CPACK_DEBIAN_MARKDOWN )
   message( FATAL_ERROR "markdown not found, required for cpack -G External -D DEB_UPLOAD_PPA=true" )
endif()

find_program( CPACK_DEBIAN_DOCBOOK_TO_MAN docbook-to-man )
if( NOT CPACK_DEBIAN_DOCBOOK_TO_MAN )
    message( FATAL_ERROR "docbook-to-man not found, required for cpack -G DEB" )
endif()

# We create a temporary debian folder that the debhelper below run as usual.
# The final debian folder is created indipendently by cpack
message( NOTICE "Creating temporary debian folder for debhelper" )
file(COPY ${CPACK_DEBIAN_SOURCE_DIR}/packaging/debian
    DESTINATION ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME})
set(CPACK_DEBIAN_PACKAGE_BUILD_DEPENDS_EXTRA "libavcodec-dev, libavutil-dev,")
configure_file(${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/control.in
               ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/control
               @ONLY)
file(REMOVE ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/control.in)

file(COPY ${CPACK_DEBIAN_SOURCE_DIR}/res/linux/mixxx-usb-uaccess.rules
    DESTINATION ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian)
file(RENAME
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/mixxx-usb-uaccess.rules
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian/mixxx.mixxx-usb-uaccess.udev)

execute_process(
    COMMAND ${CPACK_DEBIAN_DOCBOOK_TO_MAN} debian/mixxx.sgml
    OUTPUT_FILE mixxx.1
    WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
)

execute_process(
    COMMAND ${CPACK_DEBIAN_MARKDOWN} ${CPACK_DEBIAN_SOURCE_DIR}/CHANGELOG.md
    OUTPUT_FILE NEWS.html
    WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian
)

if(DEB_BUILD)
  execute_process(
    COMMAND lsb_release --short --codename
    OUTPUT_VARIABLE BUILD_MACHINE_RELEASE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

execute_process(COMMAND ${CPACK_DEBIAN_DEBCHANGE} -v "${CPACK_DEBIAN_UPSTREAM_VERSION}-${CPACK_DEBIAN_DEBIAN_VERSION}~${CPACK_DEBIAN_DEBIAN_VERSION_EXTRA}~${RELEASE}" -M "Build of ${CPACK_DEBIAN_UPSTREAM_VERSION}"
    WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME})
execute_process(COMMAND ${CPACK_DEBIAN_DEBCHANGE} -r -D ${RELEASE} -M "Build of ${CPACK_DEBIAN_UPSTREAM_VERSION}"
    WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME})

function(run_dh DH_COMMAND)
    execute_process(COMMAND ${DH_COMMAND} ${ARGV1} ${ARGV2} -P.
        WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}
        RESULT_VARIABLE CPACK_DEBIAN_DH_RET)
    if(NOT CPACK_DEBIAN_DH_RET EQUAL "0")
        message(FATAL_ERROR "${DH_COMMAND} retuned exit code ${CPACK_DEBIAN_DH_RET}")
    endif()
endfunction()

# We don't need root, normally read as Rules-Requires-Root from debian/control
set(ENV{DEB_RULES_REQUIRES_ROOT} no)

run_dh(dh_testdir)
run_dh(dh_testroot)
run_dh(dh_installdocs)
run_dh(dh_installchangelogs)
run_dh(dh_installman)
run_dh(dh_installudev --name=mixxx-usb-uaccess --priority=69)

# Remove temporary files only needed by debhelpers
file (REMOVE_RECURSE ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/debian)
file (REMOVE ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}/mixxx.1)
