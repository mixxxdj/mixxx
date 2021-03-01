cmake_minimum_required(VERSION 3.13)
include(CMakePushCheckState)
include(CheckStructHasMember)
include(CheckSymbolExists)

project(shout_mixxx LANGUAGES C VERSION 2.4.1)

add_library(${PROJECT_NAME} STATIC
    src/codec_opus.c
    src/codec_vorbis.c
    src/common/avl/avl.c
    src/common/net/sock.c
    src/common/net/resolver.c
    src/common/timing/timing.c
    src/common/httpp/httpp.c
    src/common/httpp/encoding.c
    src/common/thread/thread.c
    src/format_mpeg.c
    src/format_ogg.c
    src/format_webm.c
    src/proto_http.c
    src/proto_icy.c
    src/proto_roaraudio.c
    src/proto_xaudiocast.c
    src/queue.c
    src/shout.c
    src/tls.c
    src/util.c
)
target_include_directories(${PROJECT_NAME} PRIVATE include src src/common)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        VERSION="${PROJECT_VERSION}"
        LIBSHOUT_MAJOR=${PROJECT_VERSION_MAJOR}
        LIBSHOUT_MINOR=${PROJECT_VERSION_MINOR}
        LIBSHOUT_MICRO=${PROJECT_VERSION_PATCH}
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

cmake_push_check_state()

# Ensure we have pthreads
if(WIN32)
    # vcpkg provides CMake targets for pthreads4w
    # This won't work if pthreads4w was built without vcpkg.
    find_package(pthreads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE PThreads4W::PThreads4W)

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        __CLEANUP_C _TIMESPEC_DEFINED _USRDLL _MBCS _LIB _WIN32)
    if(STATIC_DEPS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE PTW32_STATIC_LIB)
    endif()
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    if (NOT CMAKE_USE_PTHREADS_INIT)
        message (FATAL_ERROR "Building ${PROJECT_NAME} requires the pthread library and its development heraders")
    endif ()
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
endif()

# Configure shout.h
set(SHOUT_THREADSAFE 1)
set(SHOUT_TLS 1)
configure_file(include/shoutidjc/shout.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/shoutidjc/shout.h" @ONLY)

# Required system dependencies
# Ogg Vorbis
find_package(Ogg REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Ogg::ogg)

find_package(Vorbis REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Vorbis::vorbis Vorbis::vorbisenc)

# Optional system dependencies
find_package(OpenSSL)
if(TARGET OpenSSL::SSL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_OPENSSL)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

option(SPEEX "Speex support" OFF)
if(SPEEX)
    target_sources(${PROJECT_NAME} src/codec_speex.c)
    # TODO: Add support for finding speex headers and linking
endif()

option(THEORA "Theora support" OFF)
if(THEORA)
    target_sources(${PROJECT_NAME} src/codec_theora.c)
    # TODO: Add support for finding theora headers and linking
    #find_package(unofficial-theora)
    #if(TARGET unofficial::theora::theora)
    #    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_THEORA)
    #    target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::theora::theora unofficial::theora::theoradec unofficial::theora::theoraenc)
    #endif()
endif()

check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)
if (NOT _GNU_SOURCE)
    unset(_GNU_SOURCE CACHE)
    check_symbol_exists(_GNU_SOURCE "features.h" _GNU_SOURCE)
endif ()
if (_GNU_SOURCE)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _GNU_SOURCE)
endif ()

if(UNIX AND NOT APPLE)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=600)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _XOPEN_SOURCE=600)
endif()

check_include_file(arpa/inet.h HAVE_ARPA_INET_H)
if(HAVE_ARPA_INET_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_ARPA_INET_H)
endif()

check_include_file(inttypes.h HAVE_INTTYPES_H)
if(HAVE_INTTYPES_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_INTTYPES_H)
endif()

check_include_file(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STDINT_H)
endif()

check_include_file(sys/select.h HAVE_SYS_SELECT_H)
if(HAVE_SYS_SELECT_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SYS_SELECT_H)
endif()

check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
if(HAVE_SYS_SOCKET_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SYS_SOCKET_H)
endif()

check_include_file(winsock2.h HAVE_WINSOCK2_H)
if(HAVE_WINSOCK2_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_WINSOCK2_H)
endif()

check_include_file(sys/uio.h HAVE_SYS_UIO_H)
if(HAVE_SYS_UIO_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SYS_UIO_H)
     
    check_symbol_exists(writev "sys/uio.h" HAVE_WRITEV)
    if(HAVE_WRITEV)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_WRITEV)
    endif()
endif()

check_include_file(sys/time.h HAVE_SYS_TIME_H)
if(HAVE_SYS_TIME_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SYS_TIME_H)

    check_symbol_exists(gettimeofday "sys/time.h" HAVE_GETTIMEOFDAY)
    if(HAVE_GETTIMEOFDAY)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_GETTIMEOFDAY)
    endif()
endif()

check_include_file(sys/timeb.h HAVE_SYS_TIMEB_H)
if(HAVE_SYS_TIMEB_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SYS_TIMEB_H)

    check_symbol_exists(ftime "sys/timeb.h" HAVE_FTIME)
    if(HAVE_FTIME)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FTIME)
    endif()
endif()

check_include_file(time.h HAVE_TIME_H)
if(HAVE_TIME_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_TIME_H)
    check_struct_has_member( "struct timespec" "tv_sec" "time.h" HAVE_STRUCT_TIMESPEC )
    # HAVE_STRUCT_TIMESPEC must be defined for windows pthreads on newer MSVC
    if(HAVE_STRUCT_TIMESPEC)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STRUCT_TIMESPEC)
    endif(HAVE_STRUCT_TIMESPEC)
endif()

if(HAVE_SYS_TIME_H AND HAVE_TIME_H)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TIME_WITH_SYS_TIME)
endif()

check_symbol_exists(pthread_spin_init "sys/time.h" HAVE_PTHREAD_SPIN_LOCK)
if(HAVE_PTHREAD_SPIN_LOCK)
    add_compile_definitions(HAVE_PTHREAD_SPIN_LOCK)
endif()

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    # PUBLIC because it is also used in the header file
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_UNISTD_H)
endif()

check_include_file(poll "poll.h" HAVE_POLL)
if(HAVE_POLL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_POLL)
endif()

check_include_file(winsock2.h HAVE_WINSOCK2_H)
if(HAVE_WINSOCK2_H)
    # PUBLIC because it is also used in the header file
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_WINSOCK2_H)

    check_symbol_exists(endhostent "winsock2.h;ws2tcpip.h" HAVE_ENDHOSTENT)
    check_symbol_exists(getaddrinfo "winsock2.h;ws2tcpip.h" HAVE_GETADDRINFO)
    check_symbol_exists(getnameinfo "winsock2.h;ws2tcpip.h" HAVE_GETNAMEINFO)
    check_symbol_exists(inet_aton "winsock2.h;ws2tcpip.h" HAVE_INET_ATON)
    check_symbol_exists(inet_pton "winsock2.h;ws2tcpip.h" HAVE_INET_PTON)
    check_symbol_exists(sethostent "winsock2.h;ws2tcpip.h" HAVE_SETHOSTENT)
else()
    check_symbol_exists(endhostent "netdb.h" HAVE_ENDHOSTENT)
    check_symbol_exists(getaddrinfo "sys/types.h;sys/socket.h;netdb.h" HAVE_GETADDRINFO)
    check_symbol_exists(getnameinfo "sys/socket.h;netdb.h" HAVE_GETNAMEINFO)
    check_symbol_exists(inet_aton "sys/socket.h;netinet/in.h;arpa/inet.h" HAVE_INET_ATON)
    check_symbol_exists(inet_pton "arpa/inet.h" HAVE_INET_PTON)
    check_symbol_exists(sethostent "netdb.h" HAVE_SETHOSTENT)
endif()

if(HAVE_GETADDRINFO)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_GETADDRINFO)
endif()
if(HAVE_GETNAMEINFO)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_GETNAMEINFO)
endif()
if(HAVE_INET_ATON)
    # PUBLIC because it is also used in the header file
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_INET_ATON)
endif()
if(HAVE_INET_PTON)
    # PUBLIC because it is also used in the header file
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_INET_PTON)
endif()
if(HAVE_SETHOSTENT)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SETHOSTENT)
endif()

check_symbol_exists(strcasestr "string.h" HAVE_STRCASESTR)
if(HAVE_STRCASESTR)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STRCASESTR)
endif()

check_symbol_exists(X509_check_host "openssl/x509v3.h" HAVE_X509_CHECK_HOST)
if(HAVE_X509_CHECK_HOST)
    target_compile_definitions(${PROJECT_NAME} PRIVATE XXX_HAVE_X509_check_host)
endif()

cmake_pop_check_state()
