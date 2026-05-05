---
id: BNG-29
title: "Reorganize Bungee patches to cmake/patches/ (deferred)"
status: open
priority: low
effort: small
group: BNG
package: root
labels: [bungee, cmake, deferred, refactor]
blocked_by: []
created: '2026-05-05'
---

## BNG-29: Reorganize Bungee patches to `cmake/patches/bungee/` (deferred)

## Status: deferred

This ticket captures a future-proofing reorganization that is **not currently needed**. It exists so the idea isn't lost.

## Context

The four reused Bungee tree patches
(`cmake-use-vcpkg-deps-and-install-layout.patch`, `pffft-include-path.patch`,
`assert-win32-compat.patch`, `resample-msvc-noinline.patch`) currently live in
`cmake/vcpkg-overlay-ports/bungee/` and are consumed by two paths:

1. The vcpkg overlay's `portfile.cmake` (`PATCHES` list, basename only).
2. The ExternalProject_Add path via `cmake/patches/bungee/apply-patches.cmake`, which reaches into the overlay dir via the `MIXXX_BUNGEE_OVERLAY_DIR` variable.

This design has **zero patch duplication** and is the simplest layout while BNG-20's in-tree vcpkg overlay exists. Reorganizing only matters if BNG-20 is later removed (e.g., if Mixxx maintainers prefer relying exclusively on upstream `microsoft/vcpkg` `unofficial-bungee` once it's stable).

## Trigger condition

Open this work only if/when one of:

- Mixxx maintainers request removal of `cmake/vcpkg-overlay-ports/bungee/` (BNG-20 revert or deprecation).
- A Bungee/vcpkg upstream change makes the in-tree overlay redundant.
- Cross-cutting refactor of `cmake/patches/` adds enough other movers that batching this in is cheap.

## Scope (when triggered)

- [ ] Move the four patches from `cmake/vcpkg-overlay-ports/bungee/` to `cmake/patches/bungee/`.
- [ ] Update `cmake/patches/bungee/apply-patches.cmake` to read from `MIXXX_BUNGEE_PATCH_DIR` for all five patches (the four reused + the existing `lower-cmake-minimum.patch`). Drop `MIXXX_BUNGEE_OVERLAY_DIR` from its required variable list.
- [ ] Delete `cmake/vcpkg-overlay-ports/` entirely (taking `portfile.cmake`, `vcpkg.json`, `unofficial-bungee-config.cmake`, `usage`, etc. with it). The BNG-20 in-tree overlay is being removed, so its consumers go away too.
- [ ] Update CMake's vcpkg discovery branch to drop the `set(VCPKG_OVERLAY_PORTS ...)` reference.
- [ ] Update `BNG-20.md` (in `done/`) and any docs/plans references to point at the new home.

## Acceptance criteria

- All Bungee tree patches live under `cmake/patches/bungee/`.
- `cmake/vcpkg-overlay-ports/` is deleted.
- ExternalProject path still applies all five patches in the same order.
- vcpkg discovery still finds Bungee — either via upstream `unofficial-bungee` or via whatever replacement BNG-20's removal puts in place.
- All CI matrix rows green.
