---
id: BNG-28
title: "Update GitHub Actions workflows for non-vendored Bungee"
status: done
priority: high
effort: small
group: BNG
package: root
labels: [bungee, ci, github-actions]
blocked_by: [BNG-26]
created: '2026-05-05'
completed: '2026-05-06'
---

## BNG-28: Update GitHub Actions workflows for non-vendored Bungee

## Context

Post-BNG-22 there is no `lib/bungee/` for path triggers to point at, and Bungee is built via ExternalProject (Linux dev/CI) or vcpkg (Windows/macOS) or Flatpak modules (Flatpak builder). CI workflows need their `paths:` triggers and ASan flags updated to reflect the new file layout.

This is a CI-only commit. No `CMakeLists.txt` changes.

## Scope

- [ ] **`.github/workflows/build.yml` — verify, no changes expected**
  - Confirm the Linux Ubuntu runners have unrestricted outbound network during `cmake --build` (existing `actions/setup-*` usage indicates yes).
  - Confirm `cmake --build` runs `bungee_external` + `eigen3_external` + `pffft_external` implicitly via the `mixxx-lib` dependency chain. Check by reading the actual workflow's build commands and confirming no `--target` filter would exclude them.
  - Document any proxy/firewall workaround needed (e.g., setting `BUNGEE_FETCH_FALLBACK=OFF` and relying on system Bungee). If no changes are required after verification, record that finding in the commit message.

- [ ] **`.github/workflows/bungee-asan.yml` — path triggers**
  - Change `paths:` from `lib/bungee/**` to:

    ```yaml
    paths:
      - cmake/patches/bungee/**
      - cmake/patches/pffft/**
      - cmake/vcpkg-overlay-ports/bungee/**
      - cmake/modules/FindBungee.cmake
      - src/engine/bufferscalers/enginebufferscalebungee.*
      - src/test/enginebufferscalebungeetest.cpp
      - src/test/enginebufferbungeetest.cpp
      - CMakeLists.txt   # the BUNGEE block lives here
      - .github/workflows/bungee-asan.yml
    ```

- [ ] **`.github/workflows/bungee-asan.yml` — build flags**
  - Build path uses ExternalProject. Set `-DCMAKE_BUILD_TYPE=Debug` and the Mixxx ASan flag.
  - **Verify the exact ASan flag name** by grepping `CMakeLists.txt` for `SANITIZERS\|ENABLE_ASAN\|ASAN`; do not guess. Likely `-DSANITIZERS=address` per recent CMake conventions but read the existing workflow file before changing anything.
  - ASan flags must propagate into `bungee_external` — the sub-CMake inherits `CMAKE_C_FLAGS` / `CMAKE_CXX_FLAGS` set at the parent level via `CMAKE_ARGS`. Confirm by reading how `bungee_external`'s `CMAKE_ARGS` are constructed in BNG-21's commit; if the flags aren't forwarded, add an explicit `-DCMAKE_C_FLAGS_INIT` / `-DCMAKE_CXX_FLAGS_INIT` to the `bungee_external` `CMAKE_ARGS`. Linker flags too (`-DCMAKE_EXE_LINKER_FLAGS`, `-DCMAKE_SHARED_LINKER_FLAGS`).
  - Smoke-test by triggering the workflow on a feature branch push.

## Smoke test (exit criteria)

- Push to a branch matching the workflow's `branches:` pattern with a no-op change in `cmake/patches/bungee/lower-cmake-minimum.patch` (e.g., a comment edit). The bungee-asan workflow must trigger and complete green.
- Modify a file outside the new `paths:` set (e.g., `src/dialog/dlgabout.cpp`). The bungee-asan workflow must NOT trigger (path filter works correctly).

## Acceptance criteria

- `bungee-asan.yml` triggers on Bungee-touching PRs only (verified via positive + negative test above).
- `bungee-asan.yml` builds with the correct ASan flag, propagated into `bungee_external`.
- `build.yml` Linux/macOS/Windows jobs all green with the new path. macOS + Windows resolve via vcpkg unofficial-bungee; Linux resolves via ExternalProject; Flatpak job (if separate) resolves via Flatpak modules from BNG-27.
- No changes required to `build.yml` itself — if changes ARE required, document why in the commit message.
