---
id: BNG-27
title: "Add Bungee, Eigen3, pffft as Flatpak modules"
status: done
priority: high
effort: medium
group: BNG
package: root
labels: [bungee, flatpak, packaging]
blocked_by: [BNG-26]
created: '2026-05-05'
completed: '2026-05-06'
---

## BNG-27: Add Bungee, Eigen3, pffft as Flatpak modules

## Context

Mixxx's Flatpak build is offline — `flatpak-builder` runs each module sandboxed without network during build. The ExternalProject downloads added in BNG-25/26 cannot run inside the mixxx module's build step. We must add the three deps as their own Flatpak modules ordered before mixxx in `packaging/flatpak/org.mixxx.Mixxx.yaml`.

**No CMake-side detection is required.** Mixxx's `CMakeLists.txt` has zero Flatpak-specific code. Once Bungee + Eigen3 + pffft are installed at `/app/lib/cmake/...` by their respective Flatpak modules, the BNG-21 discovery order's first step (`find_package(Bungee CONFIG QUIET)` at CMakeLists.txt:4664) succeeds and the ExternalProject branch is never entered.

## Scope

- [ ] **Create `packaging/flatpak/modules/eigen3.yaml`**
  - `buildsystem: cmake-ninja`
  - `sources: [{type: archive, url: https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz, sha256: 8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72}]`
  - `config-opts: [-DCMAKE_BUILD_TYPE=Release, -DEIGEN_BUILD_DOC=OFF, -DEIGEN_BUILD_TESTING=OFF, -DBUILD_TESTING=OFF]`
  - `cleanup: ['*']` if Eigen is build-time-only for Mixxx (verify by checking whether `org.mixxx.Mixxx.yaml`'s mixxx module needs Eigen at runtime; spec says no — it links statically into Bungee). Mirror `modules/gsl.yaml`'s pattern for header-only deps.
  - `x-checker-data` for anitya tracking (Eigen's project ID on release-monitoring.org — look it up; rubberband.yaml shows the pattern).

- [ ] **Create `packaging/flatpak/modules/pffft.yaml`**
  - Multi-source: archive + the Mixxx-supplied `CMakeLists.txt.in` from BNG-25.
  - Source 1: `{type: archive, url: https://bitbucket.org/jpommier/pffft/get/02fe7715a5bf8bfd914681c53429600f94e0f536.tar.gz, sha256: <compute>}`. Note: bitbucket archive tarballs extract into a directory named `<owner>-<repo>-<short-sha>/` — use `dest: pffft-src/` to normalize.
  - Source 2: `{type: file, path: ../../cmake/patches/pffft/CMakeLists.txt.in, dest-filename: CMakeLists.txt}` — places the Mixxx-supplied CMakeLists.txt directly in the source root so flatpak-builder's cmake-ninja step finds it.
  - `buildsystem: cmake-ninja`
  - `config-opts: [-DCMAKE_BUILD_TYPE=Release]`
  - `post-install: [- install -Dm644 -t ${FLATPAK_DEST}/share/licenses/pffft README.txt]` (or whatever LICENSE-equivalent file pffft ships).
  - **No anitya tracker** — pffft has no releases.

- [ ] **Create `packaging/flatpak/modules/bungee.yaml`**
  - Source: `{type: archive, url: https://github.com/bungee-audio-stretch/bungee/archive/refs/tags/v2.4.15.tar.gz, sha256: <compute>}`
  - Patches inline as additional `type: patch` sources, in the order `cmake/patches/bungee/apply-patches.cmake` uses:
    1. `cmake-use-vcpkg-deps-and-install-layout.patch`
    2. `pffft-include-path.patch`
    3. `assert-win32-compat.patch` (Linux build skips this in apply-patches.cmake; Flatpak is Linux-only — omit)
    4. `resample-msvc-noinline.patch` (MSVC-only — omit)
    5. `lower-cmake-minimum.patch`
  - In `flatpak.yaml` `type: patch` paths must be relative to the manifest. Reference `cmake/vcpkg-overlay-ports/bungee/cmake-use-vcpkg-deps-and-install-layout.patch` etc. via `path: ../../cmake/vcpkg-overlay-ports/bungee/...` from `packaging/flatpak/modules/`.
  - `buildsystem: cmake-ninja`
  - `config-opts: [-DCMAKE_BUILD_TYPE=Release, -DBUNGEE_VERSION=2.4.15, -DBUNGEE_PRESET=, -DBUNGEE_BUILD_SHARED_LIBRARY=ON]`
  - `x-checker-data: {type: anitya, ..., url-template: https://github.com/bungee-audio-stretch/bungee/archive/refs/tags/v$version.tar.gz}` — look up Bungee's anitya project-id, or skip if not registered.
  - `post-install: [- install -Dm644 -t ${FLATPAK_DEST}/share/licenses/bungee LICENSE]`

- [ ] **Add to `packaging/flatpak/org.mixxx.Mixxx.yaml` modules list**, ordered before `- name: mixxx`:

  ```yaml
    - modules/eigen3.yaml
    - modules/pffft.yaml
    - modules/bungee.yaml
    - name: mixxx
  ```

  Insertion point is between `modules/libdjinterop.yaml` and the inline `- name: mixxx` block.

- [ ] **Verify mixxx module config** still works — the existing `org.mixxx.Mixxx.yaml` mixxx module doesn't pass `-DBUNGEE=ON` because the option defaults to ON (CMakeLists.txt:4635). Double-check the discovery order finds the Flatpak-installed Bungee at `/app/lib/cmake/Bungee/BungeeConfig.cmake` — if it doesn't, the module install path needs to be aligned (vcpkg's `unofficial-bungee` installs `unofficial::bungee::bungee`; our Flatpak Bungee with the cmake-use-vcpkg-deps patch should install `Bungee::Bungee`).

## Smoke test (exit criteria)

On a Flatpak-capable Linux box:

```bash
cd packaging/flatpak
flatpak-builder --user --force-clean --install-deps-from=flathub --repo=test-repo build-dir org.mixxx.Mixxx.yaml
flatpak install --user test-repo org.mixxx.Mixxx
flatpak run org.mixxx.Mixxx --version  # must succeed
```

The build log must show eigen3 + pffft + bungee modules building before the mixxx module, and the mixxx module must report `Found Bungee via ...` (not `Bungee: building from upstream tarball`).

If no Flatpak builder is available locally, validate via the Flatpak CI job after pushing.

## Acceptance criteria

- `flatpak-builder` produces a working Mixxx flatpak with Bungee enabled.
- The mixxx module's CMake configure log shows Bungee resolved via `find_package` (not via ExternalProject_Add).
- All three new modules have license install commands and (where applicable) anitya checkers.
- No new env-var detection added to Mixxx's `CMakeLists.txt`.
- The Flatpak CI job (in `.github/workflows/build.yml` or whichever workflow runs the Flatpak path — verify) is green.
