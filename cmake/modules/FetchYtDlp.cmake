# FetchYtDlp.cmake
#
# Downloads the official yt-dlp standalone binary at configure time and
# exposes its on-disk path as the cache variable ${YTDLP_BINARY_PATH}, so
# the install() step can ship it next to the `mixxx` executable.
#
# Why bundle yt-dlp at all: the YouTube/Spotify library features in this fork
# need a maintained YouTube extractor. Hand-rolling InnerTube against Google
# breaks within months because the per-client API keys, signing schemes and
# PoToken anti-bot guard rotate constantly. yt-dlp ships a self-contained
# PyInstaller binary (no Python on the target machine, no shared libraries,
# Unlicense / public-domain) precisely for embedding scenarios like this, so
# bundling it is the smallest possible change that gives users a YouTube tab
# that "just works" with no external setup.
#
# Pinned to a known-good release; bump YTDLP_VERSION + the per-platform
# SHA-256 from the matching SHA2-256SUMS asset on GitHub Releases:
#   https://github.com/yt-dlp/yt-dlp/releases
#
# Desktop (Win/macOS/Linux): bundles the standard PyInstaller binary.
#
# Android: bundles a static Python 3.11 musl/aarch64 binary + the yt-dlp
# zipimport package.  PyInstaller binaries cannot run under Bionic, so we
# build yt-dlp from the universal zip and invoke it via an embedded Python
# interpreter at runtime.  The assets are installed into the APK's
# lib/extraResources/ folder so the C++ layer can discover them at runtime.

set(
  YTDLP_VERSION
  "2026.03.17"
  CACHE STRING
  "Pinned yt-dlp release tag to bundle inside Mixxx"
)

# ---------------------------------------------------------------------------
# Platform-specific yt-dlp binary (PyInstaller) for desktop
# ---------------------------------------------------------------------------
if(WIN32)
  set(_ytdlp_asset "yt-dlp.exe")
  set(
    _ytdlp_sha256
    "3db811b366b2da47337d2fcfdfe5bbd9a258dad3f350c54974f005df115a1545"
  )
  set(_ytdlp_install_name "yt-dlp.exe")
elseif(APPLE)
  set(_ytdlp_asset "yt-dlp_macos")
  set(
    _ytdlp_sha256
    "e80c47b3ce712acee51d5e3d4eace2d181b44d38f1942c3a32e3c7ff53cd9ed5"
  )
  set(_ytdlp_install_name "yt-dlp")
elseif(UNIX)
  if(ANDROID)
    # Android: skip the desktop downloader entirely.
    # The youtubedl-android AAR is handled below in the Android block.
  else()
    set(_ytdlp_asset "yt-dlp_linux")
    set(
      _ytdlp_sha256
      "c2b0189f581fe4a2ddd41954f1bcb7d327db04b07ed0dea97e4f1b3e09b5dd8e"
    )
    set(_ytdlp_install_name "yt-dlp")
  endif()
else()
  message(STATUS "yt-dlp bundling skipped: unsupported platform")
  set(
    YTDLP_BINARY_PATH
    ""
    CACHE FILEPATH
    "yt-dlp binary to install with Mixxx"
    FORCE
  )
  return()
endif()

# Desktop downloader: skip entirely on Android.
if(NOT ANDROID)
  set(_ytdlp_cache_dir "${CMAKE_BINARY_DIR}/_deps/yt-dlp-${YTDLP_VERSION}")
  set(_ytdlp_path "${_ytdlp_cache_dir}/${_ytdlp_install_name}")
  set(
    _ytdlp_url
    "https://github.com/yt-dlp/yt-dlp/releases/download/${YTDLP_VERSION}/${_ytdlp_asset}"
  )

  file(MAKE_DIRECTORY "${_ytdlp_cache_dir}")

  # If a previous configure already produced a verified copy, reuse it. This
  # keeps incremental configures cheap and lets users seed the cache offline.
  set(_need_download TRUE)
  if(EXISTS "${_ytdlp_path}")
    file(SHA256 "${_ytdlp_path}" _have_sha)
    if(_have_sha STREQUAL _ytdlp_sha256)
      set(_need_download FALSE)
      message(STATUS "Using cached yt-dlp ${YTDLP_VERSION} at ${_ytdlp_path}")
    else()
      message(
        STATUS
        "Cached yt-dlp at ${_ytdlp_path} has wrong SHA-256, re-downloading"
      )
      file(REMOVE "${_ytdlp_path}")
    endif()
  endif()

  if(_need_download)
    # Match the existing prebuilt-deps download retry pattern used in
    # CMakeLists.txt — GitHub release CDN occasionally returns 0-byte bodies
    # or 5xx, and a single failed configure is hard to debug.
    set(_max_attempts 3)
    set(_attempt 1)
    set(_ytdlp_ok FALSE)
    while(_attempt LESS_EQUAL _max_attempts AND NOT _ytdlp_ok)
      message(
        STATUS
        "Downloading yt-dlp ${YTDLP_VERSION} (${_ytdlp_asset}), attempt ${_attempt}/${_max_attempts}"
      )
      # Omit EXPECTED_HASH here: when the download fails (e.g. network blocked
      # in a Flatpak or offline CI runner), file(DOWNLOAD ... EXPECTED_HASH ...)
      # emits a CMake Error that marks the whole configure as failed even though
      # our fallback path is perfectly valid. Verify the hash ourselves after a
      # successful download so a _corrupted_ binary still fails loudly without
      # a network failure causing a hard configure error.
      file(
        DOWNLOAD "${_ytdlp_url}" "${_ytdlp_path}"
        SHOW_PROGRESS
        STATUS _dl_status
        TLS_VERIFY ON
      )
      list(GET _dl_status 0 _dl_code)
      list(GET _dl_status 1 _dl_msg)
      if(_dl_code EQUAL 0 AND EXISTS "${_ytdlp_path}")
        file(SIZE "${_ytdlp_path}" _dl_size)
        if(_dl_size GREATER 0)
          # Verify the hash so a CDN glitch that returns garbage doesn't sneak
          # into a shipped binary. A hash mismatch is an unexpected hard error
          # (download infrastructure broken), not a soft network-blocked case.
          file(SHA256 "${_ytdlp_path}" _actual_sha)
          if(_actual_sha STREQUAL _ytdlp_sha256)
            set(_ytdlp_ok TRUE)
          else()
            message(
              WARNING
              "yt-dlp download hash mismatch (expected ${_ytdlp_sha256}, got ${_actual_sha}), retrying"
            )
            file(REMOVE "${_ytdlp_path}")
          endif()
        else()
          message(STATUS "Downloaded yt-dlp is 0 bytes, retrying")
          file(REMOVE "${_ytdlp_path}")
        endif()
      else()
        message(STATUS "yt-dlp download failed: ${_dl_msg}")
        file(REMOVE "${_ytdlp_path}")
      endif()
      math(EXPR _attempt "${_attempt} + 1")
    endwhile()

    if(NOT _ytdlp_ok)
      message(
        WARNING
        "Failed to download yt-dlp ${YTDLP_VERSION} from ${_ytdlp_url} after "
        "${_max_attempts} attempts. The YouTube tab will fall back to a "
        "yt-dlp on the user's PATH or pointed at by MIXXX_YTDLP. To bundle a "
        "binary, place ${_ytdlp_asset} at ${_ytdlp_path} (or fix network "
        "access) and re-run cmake."
      )
      set(
        YTDLP_BINARY_PATH
        ""
        CACHE FILEPATH
        "yt-dlp binary to install with Mixxx"
        FORCE
      )
      return()
    endif()
  endif()

  # Make the Linux / macOS binary executable. Windows .exe needs no chmod.
  if(NOT WIN32)
    file(
      CHMOD
      "${_ytdlp_path}"
      PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
    )
  endif()

  set(
    YTDLP_BINARY_PATH
    "${_ytdlp_path}"
    CACHE FILEPATH
    "yt-dlp binary to install with Mixxx"
    FORCE
  )
  message(
    STATUS
    "Mixxx will bundle yt-dlp ${YTDLP_VERSION} from ${YTDLP_BINARY_PATH}"
  )
endif()

# ---------------------------------------------------------------------------
# Android: bundle youtubedl-android AAR (Python 3.11 runtime + yt-dlp)
# ---------------------------------------------------------------------------
if(ANDROID)
  set(
    _ytdlp_android_dir
    "${CMAKE_BINARY_DIR}/_deps/yt-dlp-android-${YTDLP_VERSION}"
  )
  file(MAKE_DIRECTORY "${_ytdlp_android_dir}")

  # Download the youtubedl-android AAR which bundles:
  #   - Python 3.11 runtime as JNI shared libs (libpython.so + libpython.zip.so)
  #   - yt-dlp Python package as a raw resource (res/raw/ytdlp)
  #   - Java/Kotlin wrapper classes (classes.jar)
  # Maven coordinates: io.github.junkfood02.youtubedl-android:library:0.18.1
  set(_ytdlp_aar_path "${_ytdlp_android_dir}/library-0.18.1.aar")
  set(
    _ytdlp_aar_url
    "https://repo1.maven.org/maven2/io/github/junkfood02.youtubedl-android/library/0.18.1/library-0.18.1.aar"
  )
  # SHA-256 of the AAR (verified against Maven Central).
  set(
    _ytdlp_aar_sha256
    "579b5fb480892b1abc2b218c2089699d52759cc8d7ba256bf876453f0365faef"
  )

  # AAR is ~57 MB (mostly the Python stdlib zip embedded in libpython.zip.so).
  # Check cache first to avoid re-downloading on every configure.
  set(_need_aar_download TRUE)
  if(EXISTS "${_ytdlp_aar_path}")
    file(SIZE "${_ytdlp_aar_path}" _aar_size)
    if(_aar_size GREATER 50000000)
      file(SHA256 "${_ytdlp_aar_path}" _aar_have_sha)
      if(_aar_have_sha STREQUAL _ytdlp_aar_sha256)
        set(_need_aar_download FALSE)
        message(
          STATUS
          "Using cached youtubedl-android AAR (${_aar_size} bytes, SHA256 verified)"
        )
      else()
        message(
          WARNING
          "Cached youtubedl-android AAR has wrong SHA256 (${_aar_have_sha}), re-downloading"
        )
        file(REMOVE "${_ytdlp_aar_path}")
      endif()
    else()
      file(REMOVE "${_ytdlp_aar_path}")
    endif()
  endif()

  if(_need_aar_download)
    # AAR is ~57 MB. Maven Central CDN or GitHub Actions runners can be flaky,
    # so retry up to 3 times before giving up.
    set(_max_attempts 3)
    set(_attempt 1)
    set(_aar_ok FALSE)
    while(_attempt LESS_EQUAL _max_attempts AND NOT _aar_ok)
      message(STATUS "Downloading youtubedl-android AAR (${_attempt}/${_max_attempts})...")
      file(
        DOWNLOAD "${_ytdlp_aar_url}" "${_ytdlp_aar_path}"
        SHOW_PROGRESS
        STATUS _dl_status
        TLS_VERIFY ON
      )
      list(GET _dl_status 0 _dl_code)
      list(GET _dl_status 1 _dl_msg)
      if(_dl_code EQUAL 0 AND EXISTS "${_ytdlp_aar_path}")
        file(SIZE "${_ytdlp_aar_path}" _aar_size)
        if(_aar_size GREATER 50000000)
          # Verify SHA256 so a CDN glitch that returns garbage doesn't sneak in.
          file(SHA256 "${_ytdlp_aar_path}" _aar_dl_sha)
          if(_aar_dl_sha STREQUAL _ytdlp_aar_sha256)
            set(_aar_ok TRUE)
          else()
            message(STATUS "SHA256 mismatch, retrying")
            file(REMOVE "${_ytdlp_aar_path}")
          endif()
        else()
          message(STATUS "Downloaded file too small (${_aar_size} bytes), retrying")
          file(REMOVE "${_ytdlp_aar_path}")
        endif()
      else()
        message(STATUS "Download failed: ${_dl_msg}, retrying")
        file(REMOVE "${_ytdlp_aar_path}")
      endif()
      math(EXPR _attempt "${_attempt} + 1")
    endwhile()

    if(NOT _aar_ok)
      message(
        FATAL_ERROR
        "Failed to download youtubedl-android AAR after ${_max_attempts} attempts. "
        "You can manually place the AAR at ${_ytdlp_aar_path} and re-run cmake."
      )
    endif()
    file(SIZE "${_ytdlp_aar_path}" _aar_size)
    message(
      STATUS
      "Downloaded and verified youtubedl-android AAR (${_aar_size} bytes)"
    )
  endif()

  # Extract the AAR (it's a zip file).
  set(_ytdlp_aar_extracted "${_ytdlp_android_dir}/extracted")
  file(MAKE_DIRECTORY "${_ytdlp_aar_extracted}")

  # We use CMake's execute_process to extract since file(ARCHIVE_EXTRACT)
  # is only available in CMake 3.18+.
  if(NOT EXISTS "${_ytdlp_aar_extracted}/jni")
    message(STATUS "Extracting youtubedl-android AAR...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xf "${_ytdlp_aar_path}"
      WORKING_DIRECTORY "${_ytdlp_aar_extracted}"
      RESULT_VARIABLE _extract_result
    )
    if(NOT _extract_result EQUAL 0)
      message(FATAL_ERROR "Failed to extract youtubedl-android AAR")
    endif()
  endif()

  # ---------------------------------------------------------------------------
  # Install JNI shared libraries into the APK's lib directory.
  # These go into QT_ANDROID_EXTRA_LIBS so Qt's androiddeployqt picks them up.
  # ---------------------------------------------------------------------------
  set(_ytdlp_jni_libs "")
  foreach(_abi IN ITEMS arm64-v8a armeabi-v7a x86_64)
    set(_aar_jni_dir "${_ytdlp_aar_extracted}/jni/${_abi}")
    if(EXISTS "${_aar_jni_dir}")
      # Collect all .so files from this ABI's jni directory.
      file(GLOB _abi_so_files "${_aar_jni_dir}/*.so")
      foreach(_so_file IN LISTS _abi_so_files)
        list(APPEND _ytdlp_jni_libs "${_so_file}")
        message(STATUS "  yt-dlp JNI lib: ${_so_file}")
      endforeach()
    endif()
  endforeach()

  # ---------------------------------------------------------------------------
  # Install the classes.jar into the APK's classpath.
  # We add it to the target's linked libraries so androiddeployqt includes it.
  # ---------------------------------------------------------------------------
  set(_ytdlp_classes_jar "${_ytdlp_aar_extracted}/classes.jar")
  if(EXISTS "${_ytdlp_classes_jar}")
    message(STATUS "  yt-dlp classes.jar: ${_ytdlp_classes_jar}")
  else()
    message(FATAL_ERROR "classes.jar not found in youtubedl-android AAR")
  endif()

  # ---------------------------------------------------------------------------
  # Install the yt-dlp package as an Android asset.
  # The youtubedl-android library loads it from res/raw/ytdlp at runtime.
  # ---------------------------------------------------------------------------
  set(_ytdlp_raw "${_ytdlp_aar_extracted}/res/raw/ytdlp")
  if(EXISTS "${_ytdlp_raw}")
    install(
      FILES "${_ytdlp_raw}"
      DESTINATION "${CMAKE_SOURCE_DIR}/packaging/android/assets"
      # androiddeployqt packages everything under the assets directory
      # into the APK's assets folder.
      RENAME "ytdlp"
    )
    message(STATUS "  yt-dlp package: ${_ytdlp_raw}")
  else()
    message(FATAL_ERROR "res/raw/ytdlp not found in youtubedl-android AAR")
  endif()

  # ---------------------------------------------------------------------------
  # Add JNI libs and classes.jar to the target.
  # ---------------------------------------------------------------------------
  set_property(
    TARGET mixxx
    APPEND
    PROPERTY QT_ANDROID_EXTRA_LIBS ${_ytdlp_jni_libs}
  )

  # Add classes.jar to the target's dependencies.
  target_link_libraries(mixxx PRIVATE "${_ytdlp_classes_jar}")

  # ---------------------------------------------------------------------------
  # Define a preprocessor macro so the C++ code knows the bundled runtime exists.
  # ---------------------------------------------------------------------------
  target_compile_definitions(mixxx-lib PUBLIC HAVE_YTDLP_ANDROID=1)

  message(STATUS "youtubedl-android AAR bundled for Android")
endif()
