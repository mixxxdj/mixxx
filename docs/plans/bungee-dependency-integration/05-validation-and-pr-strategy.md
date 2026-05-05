# 05 — Validation and PR Strategy

## 5.1 Suggested PR sequence

Prefer small reviewable PRs rather than one large vendor-removal PR.

### PR 1 — Upstream/package groundwork

- verify Bungee upstream CMake/install fixes still needed after Microsoft vcpkg PR #50120;
- verify MSVC compatibility status against the vcpkg port patches (`assert-win32-compat.patch`, `resample-msvc-noinline.patch`);
- ensure the Mixxx vcpkg fork/buildenv contains the merged upstream `bungee` port, either by baseline update, cherry-pick/backport, or approved overlay.

### PR 2 — Mixxx CMake dependency path

- improve `FindBungee.cmake`;
- add `find_package(Bungee)` path;
- optionally add `ExternalProject_Add` fallback if approved;
- keep current functionality guarded by `BUNGEE`.

### PR 3 — Remove vendored source

- remove `lib/bungee/`;
- remove direct `add_library(bungee...)` code;
- update docs to describe dependency workflow.

### PR 4 — Packaging/CI enablement

- Flatpak module;
- Debian/package updates;
- buildenv artifact updates;
- CI defaults.

PRs can be combined if maintainers prefer, but this sequence reduces review risk.

All PR work should ultimately be backported/cherry-picked into `feature/bungee` as described in [06-branch-ci-discipline.md](06-branch-ci-discipline.md). Do not mark the branch ready if any commit on `feature/bungee` is known to fail required CI.

## 5.2 Functional validation

Run or request CI for every commit on `feature/bungee`, not only the final branch head:

- `-DBUNGEE=ON` build with package/vcpkg dependency;
- `-DBUNGEE=OFF` build;
- QML enabled and disabled if relevant;
- Debug and Release/RelWithDebInfo;
- Linux, Windows, macOS;
- Android/iOS if enabled.

## 5.3 Audio/regression validation

Run existing tests:

- `src/test/enginebufferbungeetest.cpp`
- `src/test/enginebufferscalebungeetest.cpp`

Manual smoke tests:

- select Bungee keylock engine in Preferences;
- play forward/backward;
- seek while keylock is active;
- change tempo significantly;
- hit start/end of track;
- compare against Rubber Band/SoundTouch for obvious artifacts;
- verify no allocations introduced in steady-state scaler path.

## 5.4 Build-system validation

Check for every commit on `feature/bungee`:

- clean configure from empty build dir;
- incremental rebuild after dependency target exists;
- Ninja and Unix Makefiles if possible;
- no source tree dirtying after configure/build;
- offline build behavior when archive is already in `downloads/`;
- correct failure message if Bungee unavailable and fallback disabled.

## 5.5 Maintainer-facing rationale

The PR description should emphasize:

- Microsoft vcpkg PR #50120 already added Bungee `2.4.15` and passed upstream vcpkg CI;
- Mixxx either consumes that port directly or documents any backport/overlay choice;
- every commit on `feature/bungee` has been validated and CI-green;

- no vendored Bungee/Eigen/PFFFT source in Mixxx;
- dependency follows existing Mixxx patterns;
- pinned versions and hashes where network fallback exists;
- platform packaging story is explicit;
- `BUNGEE=OFF` remains available;
- Bungee integration code remains isolated to the engine scaler.
