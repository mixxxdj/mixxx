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
  "2026.08.19"
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
    "66674953fe251b89f4d08c5f0e35e0728679bd67ab3d7d05c0562af101dd3e7a"
  )
  set(_ytdlp_install_name "yt-dlp.exe")
elseif(APPLE)
  set(_ytdlp_asset "yt-dlp_macos")
  set(
    _ytdlp_sha256
    "0f192b7ec147ab6288885d6351d9ab67367640029b4377576ef46dd79cf7b202"
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
      "58162f9bfdc27458ea47bfcb311cf47028f17d8154a8bf7d689861d46399230a"
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
    # so retry up to 3 times per mirror before giving up. repo1 and
    # repo.maven.apache.org serve the same content from different frontends,
    # so alternate between them on each attempt.
    set(
      _ytdlp_aar_mirrors
      "https://repo1.maven.org/maven2/io/github/junkfood02.youtubedl-android/library/0.18.1/library-0.18.1.aar"
      "https://repo.maven.apache.org/maven2/io/github/junkfood02/youtubedl-android/library/0.18.1/library-0.18.1.aar"
    )
    set(_max_attempts 6)
    set(_attempt 1)
    set(_aar_ok FALSE)
    while(_attempt LESS_EQUAL _max_attempts AND NOT _aar_ok)
      math(EXPR _mirror_idx "(${_attempt} - 1) % 2")
      list(GET _ytdlp_aar_mirrors ${_mirror_idx} _ytdlp_aar_url)
      message(
        STATUS
        "Downloading youtubedl-android AAR (${_attempt}/${_max_attempts}) from ${_ytdlp_aar_url}..."
      )
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
          message(
            STATUS
            "Downloaded file too small (${_aar_size} bytes), retrying"
          )
          file(REMOVE "${_ytdlp_aar_path}")
        endif()
      else()
        message(STATUS "Download failed: ${_dl_msg}, retrying")
        file(REMOVE "${_ytdlp_aar_path}")
      endif()
      math(EXPR _attempt "${_attempt} + 1")
    endwhile()

    if(NOT _aar_ok)
      # In CI (GitHub Actions, etc.), a missing AAR means the bundled yt-dlp
      # fallback is silently broken — downstream code cannot download YouTube
      # audio at all once InnerTube gets blocked. Fail hard so the build
      # explicitly shows the error instead of silently shipping a broken APK.
      if(DEFINED ENV{CI} AND NOT "$ENV{CI}" STREQUAL "")
        message(
          FATAL_ERROR
          "Failed to download youtubedl-android AAR after ${_max_attempts} attempts. "
          "The APK will ship without the bundled yt-dlp runtime, making YouTube "
          "downloads non-functional. Check network access to Maven Central.\n"
          "  URL: ${_ytdlp_aar_url}\n"
          "  Cache path: ${_ytdlp_aar_path}"
        )
      else()
        message(
          WARNING
          "Failed to download youtubedl-android AAR after ${_max_attempts} attempts. "
          "You can manually place the AAR at ${_ytdlp_aar_path} and re-run cmake."
        )
      endif()
    endif()
  endif()

  if(EXISTS "${_ytdlp_aar_path}")
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
    # Bundle the youtubedl-android AARs whole into packaging/android/libs/.
    # Qt's androiddeployqt Gradle template declares
    #   implementation fileTree(dir: 'libs', include: ['*.jar', '*.aar'])
    # so any .aar dropped in <package-source-dir>/libs/ is compiled by Gradle
    # exactly like a normal `implementation files(...)` dependency: its
    # classes.jar + transitive jni/*.so + res/ are merged into the APK,
    # and its R class (R$raw.ytdlp) is generated by aapt. This is the
    # supported mechanism — there is no QT_ANDROID_* CMake property for jars.
    #
    # We bundle TWO AARs (both pinned to 0.18.1, matching the POMs):
    #   library-0.18.1.aar — YoutubeDL.kt + jni/arm64-v8a/*.so + res/raw/ytdlp
    #   common-0.18.1.aar  — SharedPrefsHelper, ZipUtils (init() deps)
    #
    # The remaining runtime deps come from Maven Central as plain jars into
    # the same libs/ dir (Gradle fileTree picks them up too). Versions match
    # the library POM: kotlin-stdlib 1.7.22, jackson-databind/core/annotations
    # 2.11.1, commons-io 2.5, commons-compress 1.12 (via common POM).
    #
    # Why the previous approach failed: only the bare classes.jar was copied
    # to libs/. YoutubeDL.kt imports kotlin-stdlib, jackson-databind,
    # commons-io and youtubedl_common — all absent — so the class failed
    # verification and getInstance() returned null at runtime (the
    # "YoutubeDL.getInstance() returned invalid" log symptom). The JNI .so
    # files were also deliberately skipped, so even a verified class could
    # never init() (it reads libpython.so from nativeLibraryDir).
    # ---------------------------------------------------------------------------
    # libandroid-support.so stub: libpython.so declares DT_NEEDED on this
    # pre-NDK-r23 compat shim but references zero symbols from it (verified
    # via nm -D: only Py_BytesMain + libc hooks are undefined). Without it,
    # initPython() fails: dlopen ENOENT -> init() throws "failed to
    # initialize". Build an empty stub with the NDK clang and ship it via
    # QT_ANDROID_EXTRA_LIBS alongside the AAR's own jni libs (which Gradle
    # merges from the .aar automatically).
    # ---------------------------------------------------------------------------
    set(_ytdlp_stub_c
      "${CMAKE_SOURCE_DIR}/packaging/android/stub/android_support_stub.c")
    set(_ytdlp_stub_so "${_ytdlp_android_dir}/libandroid-support.so")
    if(NOT EXISTS "${_ytdlp_stub_so}")
      message(STATUS "Building libandroid-support.so stub for youtubedl-android...")
      execute_process(
        COMMAND
          "${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/${ANDROID_NDK_HOST_SYSTEM_NAME}/bin/aarch64-linux-android${ANDROID_NATIVE_API_LEVEL}-clang"
          -shared -fPIC -O2 -o "${_ytdlp_stub_so}" "${_ytdlp_stub_c}"
        RESULT_VARIABLE _stub_result
      )
      if(NOT _stub_result EQUAL 0)
        message(FATAL_ERROR "Failed to build libandroid-support.so stub")
      endif()
    endif()
    set_property(
      TARGET mixxx
      APPEND PROPERTY QT_ANDROID_EXTRA_LIBS "${_ytdlp_stub_so}"
    )
    message(STATUS "  yt-dlp native stub: libandroid-support.so")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/packaging/android/libs")
    file(
      COPY "${_ytdlp_aar_path}"
      DESTINATION "${CMAKE_SOURCE_DIR}/packaging/android/libs"
    )
    message(STATUS "  yt-dlp AAR: library-0.18.1.aar")

    # Runtime deps: fetch common-0.18.1.aar whole (Gradle merges it) plus the
    # plain jars from Maven Central (pinned versions matching the POMs).
    # NOTE: kotlin-stdlib is deliberately NOT bundled — Qt's androiddeployqt
    # Gradle template already pulls org.jetbrains.kotlin:kotlin-stdlib
    # (1.8.22) for its own bindings, and shipping a second copy fails the
    # build at :checkReleaseDuplicateClasses.
    set(_ytdlp_dep_files
      "io/github/junkfood02/youtubedl-android/common/0.18.1/common-0.18.1.aar"
      "com/fasterxml/jackson/core/jackson-databind/2.11.1/jackson-databind-2.11.1.jar"
      "com/fasterxml/jackson/core/jackson-core/2.11.1/jackson-core-2.11.1.jar"
      "com/fasterxml/jackson/core/jackson-annotations/2.11.1/jackson-annotations-2.11.1.jar"
      "commons-io/commons-io/2.5/commons-io-2.5.jar"
      "org/apache/commons/commons-compress/1.12/commons-compress-1.12.jar"
    )
    foreach(_maven_path IN LISTS _ytdlp_dep_files)
      get_filename_component(_dest_name "${_maven_path}" NAME)
      set(_dep_cached "${_ytdlp_android_dir}/${_dest_name}")
      if(NOT EXISTS "${_dep_cached}")
        set(_dep_url "https://repo1.maven.org/maven2/${_maven_path}")
        message(STATUS "Downloading ${_dep_url}...")
        file(
          DOWNLOAD "${_dep_url}" "${_dep_cached}"
          SHOW_PROGRESS
          STATUS _dl_status
          TLS_VERIFY ON
        )
        list(GET _dl_status 0 _dl_code)
        if(NOT _dl_code EQUAL 0)
          message(
            WARNING
            "Failed to download ${_dep_url} — bundled yt-dlp will be missing ${_dest_name}"
          )
          file(REMOVE "${_dep_cached}")
        endif()
      endif()
      if(EXISTS "${_dep_cached}")
        file(
          COPY "${_dep_cached}"
          DESTINATION "${CMAKE_SOURCE_DIR}/packaging/android/libs"
        )
        message(STATUS "  yt-dlp dep: ${_dest_name}")
      endif()
    endforeach()

    # ---------------------------------------------------------------------------
    # Define a preprocessor macro so the C++ code knows the bundled runtime exists.
    # ---------------------------------------------------------------------------
    target_compile_definitions(mixxx-lib PUBLIC HAVE_YTDLP_ANDROID=1)

    message(STATUS "youtubedl-android AAR bundled for Android")
  endif()
endif()
