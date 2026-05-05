# 02 — Mixxx vcpkg Build Environment

Mixxx's Windows/macOS developer and release builds use the Mixxx-maintained vcpkg environment. Bungee should be consumed from vcpkg rather than copied into the Mixxx repository.

## 2.1 Start from Microsoft vcpkg PR #50120

Microsoft vcpkg PR [#50120](https://github.com/microsoft/vcpkg/pull/50120), `[bungee] Added new port`, merged on 2026-03-19 as merge commit `ae92331` with 16 checks passing.

Do **not** start by inventing a new Mixxx-specific port. First check whether the Mixxx vcpkg fork/buildenv already contains the upstream port.

Known upstream port details:

```json
{
  "name": "bungee",
  "version": "2.4.15",
  "description": "C++ library for time-stretching and pitch-shifting audio with high quality in realtime or offline",
  "homepage": "https://bungee.parabolaresearch.com/",
  "license": "MPL-2.0",
  "dependencies": [
    "eigen3",
    "pffft",
    {
      "name": "vcpkg-cmake",
      "host": true
    },
    {
      "name": "vcpkg-cmake-config",
      "host": true
    }
  ]
}
```

Known vcpkg usage from the merged port:

```cmake
find_package(unofficial-bungee CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::bungee::bungee)
```

The port also provides pkg-config module:

```text
libbungee
```

The merged portfile fetches `bungee-audio-stretch/bungee` at `v${VERSION}` and applies these patches:

- `cmake-use-vcpkg-deps-and-install-layout.patch`
- `pffft-include-path.patch`
- `assert-win32-compat.patch`
- `resample-msvc-noinline.patch`

These patches are important prior art for Mixxx's own MSVC compatibility concerns.

## 2.2 Decide how to bring #50120 into Mixxx's vcpkg environment

Decision path:

1. Check whether `mixxxdj/vcpkg` is already based on a Microsoft vcpkg commit after `ae92331`.
2. If yes, prefer using the existing upstream port unchanged.
3. If not, choose one of:
   - update/rebase the Mixxx vcpkg fork to a newer Microsoft vcpkg baseline that includes `ae92331`;
   - cherry-pick/backport the upstream Bungee port commit(s) into the Mixxx vcpkg fork;
   - temporarily copy the port into `overlay/ports/bungee` only if maintainers prefer minimal fork movement.

Maintainer preference is required before choosing between baseline update vs cherry-pick vs overlay.

## 2.3 Target naming in Mixxx CMake

Because the vcpkg port exposes `unofficial-bungee` and `unofficial::bungee::bungee`, Mixxx CMake needs a compatibility strategy.

Options:

1. Use vcpkg's target directly when available:

   ```cmake
   find_package(unofficial-bungee CONFIG QUIET)
   target_link_libraries(mixxx-lib PUBLIC unofficial::bungee::bungee)
   ```

2. Wrap vcpkg's target with an alias/interface target used by Mixxx:

   ```cmake
   add_library(Bungee::Bungee INTERFACE IMPORTED)
   target_link_libraries(Bungee::Bungee INTERFACE unofficial::bungee::bungee)
   ```

3. Update `FindBungee.cmake`/CMake glue so Mixxx always links `Bungee::Bungee`, regardless of whether the source is vcpkg config, system package, pkg-config, or fallback.

Preferred plan: Mixxx code links only to `Bungee::Bungee`; dependency-specific target names are normalized in CMake discovery glue.

## 2.4 Validate vcpkg target triplets

The Microsoft vcpkg PR passed upstream vcpkg checks including Android, Linux, macOS, and Windows variants. Mixxx still needs validation against Mixxx's actual buildenv triplets and build options.

Test at least the triplets Mixxx CI/build scripts use:

- `x64-windows-release`
- `arm64-windows-release`
- `x64-osx-min1100-release`
- `arm64-osx-min1100-release`
- Android triplet if Bungee is intended on Android

Record whether these use static or dynamic linkage and whether the resulting target exports everything Mixxx needs.

## 2.5 Update Mixxx buildenv references only after port works

After the vcpkg port is present and green in the Mixxx build environment:

- update the Mixxx build environment artifact version/hash through the existing buildenv process;
- update `tools/windows_buildenv.bat`, `tools/macos_buildenv.sh`, `tools/android_buildenv.sh` only if the generated dependency archive changes;
- use `tools/update_vcpkg.py` if that is the current Mixxx process for bulk buildenv version updates;
- include the exact vcpkg commit/baseline containing Bungee in the PR description.

## 2.6 Result of this phase

Exit criteria:

- Bungee is present in the Mixxx vcpkg buildenv via upstream PR #50120, a fork baseline update, or an approved backport/overlay;
- `find_package(unofficial-bungee CONFIG)` works with `MIXXX_VCPKG_ROOT`;
- Mixxx normalizes that to a stable internal target, preferably `Bungee::Bungee`;
- Windows/macOS Mixxx builds do not need `lib/bungee/` in the source tree;
- the vcpkg provenance is documented in the Mixxx PR.
