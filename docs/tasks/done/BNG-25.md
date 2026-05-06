---
id: BNG-25
title: "Add Eigen3 + pffft as ExternalProject_Add targets"
status: done
priority: high
effort: medium
group: BNG
package: root
labels: [bungee, cmake, externalproject, eigen3, pffft]
blocked_by: [BNG-21]
created: '2026-05-05'
completed: '2026-05-06'
---

## BNG-25: Add Eigen3 + pffft as ExternalProject_Add targets

## Context

BNG-21 added an `ExternalProject_Add(bungee_external)` source-fetch fallback, but it falls through to the vendored `lib/bungee/` tree when system Eigen3 or system pffft is missing — a stock Linux dev box without those packages. BNG-25 introduces unconditional Eigen3 and pffft ExternalProjects so the BUNGEE=ON path becomes self-contained on every networked builder. After BNG-25 lands, BNG-26 wires them into `bungee_external`; together they unblock BNG-22 (vendored deletion).

This ticket adds the two ExternalProject targets but does **not** wire them into `bungee_external` — that's BNG-26. Decoupling lets us verify each ExternalProject downloads and installs cleanly in isolation before depending on the result.

### Pin research (do not redo)

- **Eigen 3.4.0 stable**
  - URL: `https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz`
  - URL_HASH: `SHA256=8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72`
  - Bungee v2.4.15's submodule pointer is `c29c8001…` (Eigen master snapshot, version macros report `EIGEN_WORLD.MAJOR.MINOR = 3.4.90`). We deliberately pin to 3.4.0 stable instead because (a) Bungee's upstream CMake has zero Eigen version constraint — the master pin is convenience not requirement; (b) Mixxx convention prefers stable release tarballs (libdjinterop 0.27.0, rubberband 4.0.0); (c) Bungee uses only stable Eigen Core API + long-standing `EIGEN_RUNTIME_NO_MALLOC` machinery, all present in 3.4.0.
  - **Fallback**: if smoke-test fails (Bungee v2.4.15 does not compile against Eigen 3.4.0), pin to the exact submodule commit instead: `https://gitlab.com/libeigen/eigen/-/archive/c29c800126982c561e8d0b9255dc65474cd98de3/eigen-c29c800126982c561e8d0b9255dc65474cd98de3.tar.gz` and document the deviation. This is the explicit fallback path — verify before changing the default.

- **pffft commit `02fe7715a5bf8bfd914681c53429600f94e0f536`** (jpommier upstream from `bitbucket.org/jpommier/pffft.git`, exact pin shipped by Bungee v2.4.15 — confirmed via `git -C lib/bungee ls-tree v2.4.15:submodules`)
  - URL: `https://bitbucket.org/jpommier/pffft/get/02fe7715a5bf8bfd914681c53429600f94e0f536.tar.gz`
  - URL_HASH: `SHA256=<compute at implementation time via curl + sha256sum>`
  - **Not** `marton78/pffft` (a CMake-enabled GitHub fork). We mirror exactly what Bungee was tested with.

## Scope

- [x] **B-1: `ExternalProject_Add(eigen3_external)`**
  - Pinned per the research block above.
  - `INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/eigen3-install`.
  - Use Eigen's own CMake (`buildsystem: cmake`); `cmake --install` produces a working `Eigen3Config.cmake` exposing `Eigen3::Eigen` (header-only INTERFACE target). **No `Eigen3Config.cmake.in` template needed** — Eigen 3.4.0 ships one.
  - Skip the build step (`BUILD_COMMAND ""`) since Eigen is header-only — go straight from CONFIGURE to INSTALL.
  - Place inside the `if(NOT Bungee_FOUND AND BUNGEE_FETCH_FALLBACK)` branch in `CMakeLists.txt` (alongside the existing `bungee_external`).

- [x] **B-2: `ExternalProject_Add(pffft_external)`**
  - Pinned per the research block above.
  - `INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/pffft-install`.
  - pffft has no upstream CMake. Supply `cmake/patches/pffft/CMakeLists.txt.in` mirroring the verbatim recipe Mixxx already uses for the vendored copy (`CMakeLists.txt:4880-4896`):

    ```cmake
    cmake_minimum_required(VERSION 3.13)
    project(pffft C)

    add_library(pffft STATIC pffft.c fftpack.c)
    target_include_directories(pffft PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include>)

    if(NOT MSVC)
      target_compile_options(pffft PRIVATE
        -ffast-math -fno-finite-math-only -fno-exceptions)
    else()
      target_compile_definitions(pffft PRIVATE _USE_MATH_DEFINES)
    endif()

    set_target_properties(pffft PROPERTIES
      POSITION_INDEPENDENT_CODE ON
      EXPORT_NAME pffft)

    install(TARGETS pffft EXPORT pffft-targets ARCHIVE DESTINATION lib)
    install(FILES pffft.h fftpack.h DESTINATION include)
    install(EXPORT pffft-targets FILE pffftConfig.cmake
            NAMESPACE pffft:: DESTINATION lib/cmake/pffft)
    ```

    Mirrors Bungee upstream's own pffft recipe (verified against `bungee-audio-stretch/bungee` v2.4.15 `CMakeLists.txt`).

  - `PATCH_COMMAND` copies the template into the extracted source tree as `CMakeLists.txt`.
  - **SIMD**: do not pass `-march=` flags. Mixxx's top-level CMake already injects `-march=native` (Linux/macOS gcc/clang; CMakeLists.txt:962) or `/arch:SSE2` (MSVC; CMakeLists.txt:812). pffft.c uses standard `__SSE__` / `__ARM_NEON__` predefined macros and inherits whatever the parent build chose.

## Smoke test (exit criteria)

On a Linux host with no system Eigen3 and no system pffft (or both apt-removed for the test):

```bash
cmake -B build -DBUNGEE=ON
cmake --build build --target eigen3_external pffft_external
ls build/eigen3-install/share/eigen3/cmake/Eigen3Config.cmake  # must exist
ls build/pffft-install/lib/cmake/pffft/pffftConfig.cmake       # must exist
ls build/pffft-install/lib/libpffft.a                          # must exist
```

All three artifacts must be present. If `Eigen3Config.cmake` install path differs (some Eigen versions install to `lib/cmake/eigen3/` instead of `share/eigen3/cmake/`), document the actual path — BNG-26 needs it.

Separately, smoke-test that Bungee v2.4.15 compiles against Eigen 3.4.0 (decoupled from the wiring in BNG-26):

```bash
# In a scratch dir:
git clone -b v2.4.15 https://github.com/bungee-audio-stretch/bungee.git scratch-bungee
cd scratch-bungee
cmake -B build -DCMAKE_PREFIX_PATH=<absolute path to eigen3-install>
cmake --build build --target bungee_library
```

If this fails with template/API errors, switch to the Eigen `c29c8001` commit pin per the fallback note in the research block.

## Acceptance criteria

- `cmake -B build -DBUNGEE=ON` configures successfully on a host with no system Eigen3/pffft.
- `cmake --build build --target eigen3_external pffft_external` produces installed CMake config files for both packages.
- The smoke-test compile of upstream Bungee v2.4.15 against the installed Eigen 3.4.0 succeeds. (If it fails, switch to the c29c8001 fallback pin and re-test.)
- `cmake -B build -DBUNGEE=OFF` still configures clean.
- `BUNGEE_FETCH_FALLBACK=OFF` still bypasses both new ExternalProjects and the existing `bungee_external` (preserving the strict packaged-only path for distro packagers).
- No changes to `bungee_external` itself in this commit (that's BNG-26).
