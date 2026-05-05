# 03 — Mixxx CMake Migration

This step changes Mixxx from direct vendored compilation to dependency discovery.

## 3.1 Preserve the feature option and default

Keep the user-facing option and default it to `ON` for this PR:

```cmake
option(BUNGEE "Enable the Bungee engine for pitch-bending" ON)
```

If a platform cannot support Bungee yet, prefer an explicit platform/buildenv override with rationale over changing the global default to `OFF`. If maintainers later reject default `ON`, present the default-change decision with pros/cons before changing it.

## 3.2 Use package discovery first

Preferred structure, accounting for Microsoft vcpkg PR #50120 exposing `unofficial-bungee`:

```cmake
if(BUNGEE)
  find_package(Bungee CONFIG QUIET)

  if(NOT Bungee_FOUND)
    find_package(unofficial-bungee CONFIG QUIET)
    if(unofficial-bungee_FOUND AND TARGET unofficial::bungee::bungee)
      add_library(Bungee::Bungee INTERFACE IMPORTED)
      target_link_libraries(Bungee::Bungee INTERFACE unofficial::bungee::bungee)
      set(Bungee_FOUND TRUE)
    endif()
  endif()

  if(NOT Bungee_FOUND)
    find_package(Bungee MODULE QUIET)
  endif()

  if(Bungee_FOUND)
    target_link_libraries(mixxx-lib PUBLIC Bungee::Bungee)
  else()
    # temporary fallback, if maintainers approve
  endif()
endif()
```

Exact implementation may differ, but Mixxx-side code should ideally link only to `Bungee::Bungee` after discovery normalization.

## 3.3 Improve `FindBungee.cmake`

Current `FindBungee.cmake` may need updates:

- create a consistent imported target `Bungee::Bungee`;
- include dependency handling for static Bungee if needed;
- support vcpkg's `unofficial-bungee` config from Microsoft vcpkg PR #50120;
- support pkg-config module `libbungee` if Bungee/vcpkg installs a `.pc` file;
- avoid hardcoded sibling hints like `../bungee` unless maintainers want developer override support;
- expose `Bungee_VERSION` from config/pkg-config if available.

## 3.4 Add temporary `ExternalProject_Add` fallback if approved

While Bungee is not distro-packaged, Mixxx may accept an `ExternalProject_Add` fallback similar to `libdjinterop`/`libkeyfinder`. However, because vcpkg now has a merged Bungee port, prefer vcpkg/system package discovery for official build environments and use ExternalProject only as a maintainer-approved developer fallback.

Requirements:

- use release archive URL, not live git clone, unless maintainers request otherwise;
- include `URL_HASH`;
- use `DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/downloads"`;
- use stable `DOWNLOAD_NAME`;
- forward:
  - `CMAKE_BUILD_TYPE`
  - `CMAKE_INSTALL_PREFIX`
  - `CMAKE_PREFIX_PATH`
  - `CMAKE_FIND_ROOT_PATH`
  - `CMAKE_MODULE_PATH`
  - `CMAKE_TOOLCHAIN_FILE`
  - `CMAKE_OSX_DEPLOYMENT_TARGET`
  - `CMAKE_OSX_ARCHITECTURES`
  - `CMAKE_SYSTEM_NAME`
  - `CMAKE_SYSTEM_PROCESSOR`
- set `BUILD_BYPRODUCTS`;
- create an imported target pointing to the installed fallback artifact;
- `file(MAKE_DIRECTORY ...)` for include dirs before setting imported target include paths, matching existing Mixxx ExternalProject patterns.

## 3.5 Remove vendored direct-build code

Remove from Mixxx CMake:

- `add_library(bungee-pffft ...)` from `lib/bungee/submodules/pffft`;
- `add_library(bungee ...)` from `lib/bungee/src/*.cpp`;
- direct include of `lib/bungee/submodules/eigen`;
- configure-time `git apply` patch logic.

## 3.6 Remove vendored source from repository

Delete from Mixxx tree:

```text
lib/bungee/
```

Keep Mixxx-owned integration files:

- `src/engine/bufferscalers/enginebufferscalebungee.*`;
- tests such as `src/test/enginebufferbungeetest.cpp` and `src/test/enginebufferscalebungeetest.cpp`;
- docs updated to describe dependency flow instead of vendor flow.

## 3.7 Result of this phase

Exit criteria:

- Mixxx builds Bungee support from a package target or approved fallback;
- no checked-in Bungee/Eigen/PFFFT source remains under `lib/bungee`;
- `-DBUNGEE=OFF` still builds cleanly;
- QML and non-QML builds link consistently.
