# Bungee Dependency Integration Plan

Goal: replace the current checked-in `lib/bungee/` source copy with a dependency integration that is more likely to be accepted by Mixxx maintainers.

The target shape is:

1. Bungee is discoverable as a normal dependency (`find_package(Bungee CONFIG)` first, `FindBungee.cmake` fallback if needed).
2. Windows/macOS release build environments get Bungee through vcpkg. Microsoft vcpkg PR [#50120](https://github.com/microsoft/vcpkg/pull/50120) added the upstream `bungee` port and merged on 2026-03-19, so the plan should prefer consuming/backporting that port rather than inventing a new one.
3. Linux/developer builds can use system/vcpkg packages when available, and optionally a pinned `ExternalProject_Add` source fallback only if maintainers want one.
4. Eigen and PFFFT are no longer copied into Mixxx under `lib/bungee/submodules`; they are resolved as dependencies.
5. Mixxx-specific patches are either upstreamed to Bungee or applied only in dependency packaging/fallback build steps, not by mutating the Mixxx source tree at configure time.
6. `BUNGEE` remains default `ON` for this PR unless maintainers explicitly request otherwise.
7. The Bungee-enabled build may recommend a newer CMake for the newest Bungee release, while optionally supporting an older validated Bungee release for builders whose installed CMake is older than the newest Bungee requires.
8. All Bungee-related work from whatever branch is active at implementation time must be backported/cherry-picked into `feature/bungee`, with every commit on `feature/bungee` independently passing all required CI jobs.

## Why this is more Mixxx-like

This follows existing accepted Mixxx patterns:

- normal `find_package(...)` dependency usage for libraries that distros package;
- vcpkg build environments for Windows/macOS developer and release builds;
- `ExternalProject_Add` fallback pattern similar to `libdjinterop` and `libkeyfinder`;
- pinned source archives with hashes and `downloads/` caching for reproducible/offline-capable builds.

## Plan files

- [00-current-state.md](00-current-state.md) — current integration and issues to unwind.
- [01-upstream-bungee.md](01-upstream-bungee.md) — upstream/package-readiness work for Bungee itself.
- [02-vcpkg-buildenv.md](02-vcpkg-buildenv.md) — Mixxx vcpkg port/build environment work.
- [03-mixxx-cmake.md](03-mixxx-cmake.md) — Mixxx-side CMake migration.
- [04-packaging-ci.md](04-packaging-ci.md) — distro, Flatpak, CI, and release packaging updates.
- [05-validation-and-pr-strategy.md](05-validation-and-pr-strategy.md) — validation matrix and PR sequencing.
- [06-branch-ci-discipline.md](06-branch-ci-discipline.md) — `feature/bungee` backport/cherry-pick workflow and per-commit CI-green requirement.
- [07-execution-tickets.md](07-execution-tickets.md) — focused BNG-17 through BNG-24 ticket sequence for the remaining work.
- [maintainer-questions.md](maintainer-questions.md) — questions to ask Mixxx/Bungee maintainers before implementation.

## Known vcpkg Bungee port

Microsoft vcpkg PR [#50120](https://github.com/microsoft/vcpkg/pull/50120), `[bungee] Added new port`, was merged into `microsoft/vcpkg:master` on 2026-03-19 as merge commit `ae92331`. The port is for Bungee `2.4.15`, depends on `eigen3` and `pffft`, and exposes the vcpkg usage:

```cmake
find_package(unofficial-bungee CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::bungee::bungee)
```

The plan should therefore evaluate:

- whether the Mixxx vcpkg fork/buildenv already contains that merge;
- if not, whether to update the fork baseline or cherry-pick/backport the vcpkg port commits;
- whether Mixxx CMake should consume the vcpkg target directly or wrap it in a `Bungee::Bungee` compatibility target in `FindBungee.cmake`/CMake glue.

## Local CMake version observed

On this machine, both existing build directories were configured with CMake **3.31.11**:

- `cmake --version`: `3.31.11`
- `build/CMakeCache.txt`: `CMAKE_CACHE_*_VERSION = 3.31.11`, generator `Unix Makefiles`, command `/usr/bin/cmake`
- `build_asan/CMakeCache.txt`: `CMAKE_CACHE_*_VERSION = 3.31.11`, generator `Unix Makefiles`, command `/usr/bin/cmake`

Important: Mixxx's top-level `CMakeLists.txt` currently requires only CMake `3.21`, so the dependency plan should not rely on 3.31-only features unless maintainers explicitly agree.
