---
id: BNG-26
title: "Wire Eigen3 + pffft ExternalProjects into bungee_external; remove vendored fallthrough"
status: done
priority: high
effort: medium
group: BNG
package: root
labels: [bungee, cmake, externalproject, vendor-removal]
blocked_by: [BNG-25]
created: '2026-05-05'
completed: '2026-05-06'
---

## BNG-26: Wire Eigen3 + pffft ExternalProjects into bungee_external

## Context

BNG-25 added unconditional `eigen3_external` and `pffft_external` targets but didn't connect them to `bungee_external`. BNG-26 closes that loop: `bungee_external` becomes self-contained and the vendored fallthrough path becomes unreachable. **This is the commit that unblocks BNG-22** (vendored deletion).

After BNG-26: a stock Linux dev box (no system Eigen3, no system pffft, with `patch` installed) can `cmake -DBUNGEE=ON .. && cmake --build .` and produce a working Mixxx with Bungee enabled, end-to-end through ExternalProject.

## Scope

All changes inside the BNG-21 ExternalProject branch (`CMakeLists.txt:4715-4870`).

- [x] **Stop computing `_bungee_fetch_missing` from `find_package(Eigen3)` / `find_package(pffft)`**. Both are now provided unconditionally by BNG-25's targets, so the missing-deps detection becomes vestigial. Remove that block.

- [x] **Keep `find_program(BUNGEE_PATCH_EXECUTABLE)` check, but change its failure mode**. Pre-BNG-26 it falls through to vendored when `patch` is missing; post-BNG-26 (and post-BNG-22) there is no vendored fallback. So missing `patch` becomes a hard `message(FATAL_ERROR)` at configure time with install hints:

  ```text
  Bungee build requires GNU patch but none was found.
  Install with:
    Debian/Ubuntu:  apt install patch
    macOS:           brew install gpatch  (the system /usr/bin/patch may also work)
    Windows MSVC:    vcpkg install patch  (or use Git-for-Windows' /usr/bin/patch)
  Or disable Bungee: cmake -DBUNGEE=OFF
  Or skip the source-fetch path: cmake -DBUNGEE_FETCH_FALLBACK=OFF (requires system Bungee install)
  ```

- [x] **Add dependencies**: `add_dependencies(bungee_external eigen3_external pffft_external)`.

- [x] **Forward install dirs** into `bungee_external`'s `CMAKE_ARGS` via `CMAKE_PREFIX_PATH=${EIGEN3_INSTALL_DIR};${PFFFT_INSTALL_DIR}` (path-list-separator-aware on Windows where `;` is the list separator inside CMAKE_ARGS — use `${CMAKE_COMMAND} -E env` or escape per CMake docs).

- [x] **Make Eigen3 + pffft visible to the parent build's link line**. `Bungee::Bungee` already lists `pffft::pffft;Eigen3::Eigen` in `INTERFACE_LINK_LIBRARIES` (CMakeLists.txt:4855). At configure time these targets don't exist yet because the ExternalProjects haven't built. Two clean options:

  **Option 1 (preferred)**: declare imported targets manually pointing at predicted install paths:

  ```cmake
  add_library(Eigen3::Eigen INTERFACE IMPORTED GLOBAL)
  set_target_properties(Eigen3::Eigen PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INSTALL_DIR}/include/eigen3")
  add_dependencies(Eigen3::Eigen eigen3_external)

  add_library(pffft::pffft STATIC IMPORTED GLOBAL)
  set_target_properties(pffft::pffft PROPERTIES
    IMPORTED_LOCATION "${PFFFT_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}pffft${CMAKE_STATIC_LIBRARY_SUFFIX}"
    INTERFACE_INCLUDE_DIRECTORIES "${PFFFT_INSTALL_DIR}/include")
  add_dependencies(pffft::pffft pffft_external)
  ```

  This sidesteps `find_package`'s configure-time-only nature and keeps target wiring deterministic. **Verify the Eigen install path** — BNG-25's smoke test recorded the actual subdir (`include/eigen3` vs `include`).

  **Option 2 (alternative)**: call `find_package(Eigen3 CONFIG REQUIRED PATHS ${EIGEN3_INSTALL_DIR}/share/eigen3/cmake)` *after* the ExternalProject_Adds. This breaks at configure time of a fresh build because the package files don't exist yet — only on rebuilds after eigen3_external has been built once. Avoid.

  Use Option 1.

- [x] **Remove the entire `else()` fallthrough branch** (CMakeLists.txt:4872 onwards: "# 5. Vendored fallback — build Bungee directly from `lib/bungee/`"). The `bungee-pffft` and vendored `bungee` `add_library` calls plus the MSVC `git apply` block all go away. The discovery comment at the top of the BUNGEE block (CMakeLists.txt:4675–4680) drops step 5.

  Actually — *keep* the `else()` branch removal for **BNG-22** (so this commit and BNG-22 stay independently reviewable). BNG-26 wires up the new path; BNG-22 deletes `lib/bungee/` and the dead code together. Confirm with maintainer: would they prefer one combined commit or two?

  **Recommended**: split per the original plan — BNG-26 makes the new path self-sufficient, BNG-22 deletes the now-unused vendored block + tree. Two reviewable diffs, two clear commit messages.

## Smoke test (exit criteria)

On a Linux host with no system Eigen3 and no system pffft (apt-removed if necessary), with `patch` installed:

```bash
rm -rf build
cmake -B build -DBUNGEE=ON
cmake --build build --target Bungee::Bungee
ldd build/lib/libbungee.* 2>/dev/null || nm build/lib/libbungee.* | grep pffft  # spot-check link
cmake --build build  # full build, links into mixxx-lib
```

Must succeed. If it fails because `Eigen3::Eigen` or `pffft::pffft` resolves to wrong paths, audit Option 1's path predictions against BNG-25's recorded install paths.

Separately verify `BUNGEE=OFF` still configures clean and `BUNGEE_FETCH_FALLBACK=OFF` still produces a strict packaged-only build path.

## Acceptance criteria

- A Linux host with no system Eigen3 and no system pffft can `cmake -DBUNGEE=ON .. && cmake --build .` produce a working Mixxx with Bungee linked.
- `bungee_external` no longer references `find_package(Eigen3)` / `find_package(pffft)` to detect missing deps.
- `_bungee_fetch_missing` logic is removed.
- Missing `patch` produces a hard configure-time error with install hints.
- `BUNGEE=OFF` configures clean.
- `BUNGEE_FETCH_FALLBACK=OFF` configures clean (still requires system Bungee).
- The vendored fallthrough block (`add_library(bungee-pffft ...)`, `add_library(bungee ...)`, MSVC `git apply` patching) is **left in place** for BNG-22 to delete — BNG-26 only makes it unreachable.

## Completion notes

- Wired `bungee_external` to depend on `eigen3_external` and `pffft_external`.
- Replaced configure-time Eigen3/pffft discovery with deterministic imported targets pointing at the BNG-25 install prefixes.
- Added a compatibility `include/pffft/` install in the pffft ExternalProject recipe because Bungee's vcpkg dependency patch includes `<pffft/pffft.h>`.
- Converted missing `patch` and missing Bungee provider after discovery/fallback to hard configure-time errors; the vendored block remains present but unreachable for BNG-22 deletion.
- Validation: source-fetch configure with local Bungee discovery disabled, `bungee_external`, `mixxx-lib`, `BUNGEE=OFF`, and packaged-only `BUNGEE_FETCH_FALLBACK=OFF` configure all passed.
