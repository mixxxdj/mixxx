# 00 — Current State

## Current implementation

The current branch adds Bungee by committing the upstream repository contents under:

```text
lib/bungee/
```

Mixxx then builds Bungee directly in the top-level `CMakeLists.txt`:

- builds `bungee-pffft` from `lib/bungee/submodules/pffft/*.c`;
- builds `bungee` from explicit `lib/bungee/src/*.cpp` files;
- includes `lib/bungee`, `lib/bungee/bungee`, and `lib/bungee/submodules/eigen`;
- applies `lib/bungee/0001-MSVC-compatibility.patch` during configure on MSVC;
- links `mixxx-lib` and `mixxx-qml-lib` directly to the local `bungee` target.

There is already a `cmake/modules/FindBungee.cmake`, but it is not used by this direct-build path.

## Acceptance risks

Likely upstream review concerns:

1. **Vendored source footprint** — copies Bungee, Eigen, PFFFT, and upstream git metadata/submodule structure into Mixxx.
2. **Distro packaging friction** — distributions generally prefer system libraries or separately packaged source dependencies over embedded copies.
3. **vcpkg mismatch** — Mixxx Windows/macOS dependency workflow is built around `mixxxdj/vcpkg`, not checked-in dependency source trees.
4. **Configure-time mutation** — applying a patch into `lib/bungee` during CMake configure can dirty the source tree and makes builds less declarative.
5. **Update/review burden** — future Bungee updates become large Mixxx PR diffs instead of dependency version bumps.
6. **License/audit burden** — copied Eigen/PFFFT/Bungee source requires Mixxx-side license and security tracking.

## Desired end state

The Mixxx tree should contain only:

- Mixxx's Bungee scaler adapter code;
- a `FindBungee.cmake` module if needed;
- CMake dependency selection logic;
- packaging metadata;
- tests and documentation.

The Mixxx tree should not contain:

- `lib/bungee/` upstream source;
- copied Eigen/PFFFT source for Bungee;
- nested `.git` or `.gitmodules` from Bungee;
- configure-time patch application against checked-in vendor code.
